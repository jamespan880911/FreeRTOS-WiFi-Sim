/********************************************************************
 *  Wi-Fi Driver / Firmware Interaction Simulation (FreeRTOS)
 *  Enhanced Version: Dual Mutex (TX / RX), Locks remain in upper layer
 *
 *  Design Goal:
 *    - 模擬 Driver / Firmware 雙向資料流
 *    - 各自使用不同鎖 (xTxMutex / xRxMutex)
 ********************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ========================= 系統參數 ========================= */
#define RING_SIZE         8
#define WIFI_TX_PERIOD    300
#define FW_PROC_TIME      150
#define FW_IDLE_TIMEOUT   500

/* ========================= 資料結構 ========================= */
typedef struct {
    uint8_t  valid;
    uint32_t len;
    uint8_t *buf;
} dma_desc_t;

typedef struct {
    dma_desc_t desc[RING_SIZE];
    uint8_t head;
    uint8_t tail;
} dma_ring_t;

typedef struct {
    uint32_t seq;
    char payload[64];
} wifi_pkt_t;

/* ========================= 全域變數 ========================= */
static dma_ring_t tx_ring;  // Driver → FW
static dma_ring_t rx_ring;  // FW → Driver

static SemaphoreHandle_t xIRQ_Sem;   // 模擬 IRQ
static SemaphoreHandle_t xTxMutex;   // TX Ring 保護鎖
static SemaphoreHandle_t xRxMutex;   // RX Ring 保護鎖

static volatile uint8_t irq_masked = 0;
static volatile uint8_t fw_sleep = 0;
static uint32_t sys_time_ms = 0;

/* ========================= 輔助函式 ========================= */
static inline void print_time(const char *tag)
{
    printf("[T=%03lu ms] %s\n", (unsigned long)sys_time_ms, tag);
}

/* ========================= DMA Ring 操作 ========================= */
static int dma_push(dma_ring_t *r, uint8_t *buf, uint32_t len)
{
    uint8_t next = (r->head + 1) % RING_SIZE;
    if (next == r->tail) return -1;  // full

    r->desc[r->head].buf = buf;
    r->desc[r->head].len = len;
    r->desc[r->head].valid = 1;
    r->head = next;
    return 0;
}

static int dma_pop(dma_ring_t *r, uint8_t **buf, uint32_t *len)
{
    if (r->tail == r->head) return -1;  // empty
    if (!r->desc[r->tail].valid) return -1;

    *buf = r->desc[r->tail].buf;
    *len = r->desc[r->tail].len;
    r->desc[r->tail].valid = 0;
    r->tail = (r->tail + 1) % RING_SIZE;
    return 0;
}

/* ========================= SDIO 抽象層 ========================= */
static int sdio_write(uint8_t *data, uint32_t len) {
    return dma_push(&tx_ring, data, len);
}

static int sdio_read(uint8_t **buf, uint32_t *len) {
    return dma_pop(&rx_ring, buf, len);
}

/* ========================= Driver TX Task ========================= */
static void vWiFiDriverTx(void *p)
{
    uint32_t seq = 0;
    for (;;) {
        wifi_pkt_t *pkt = pvPortMalloc(sizeof(wifi_pkt_t));
        pkt->seq = seq++;
        snprintf(pkt->payload, sizeof(pkt->payload), "TX Packet #%lu", (unsigned long)pkt->seq);

        /* 🟦 使用 TX Mutex 保護 TX Ring */
        xSemaphoreTake(xTxMutex, portMAX_DELAY);
        if (sdio_write((uint8_t *)pkt, sizeof(wifi_pkt_t)) == 0)
            print_time("[Driver] TX → FW");
        else
            print_time("[Driver] TX Ring Full");
        xSemaphoreGive(xTxMutex);

        /* 模擬偶爾關閉中斷 (Critical Section) */
        if (seq % 5 == 0) {
            irq_masked = 1;
            print_time("[Driver] Mask IRQ (Critical Section)");
            vTaskDelay(pdMS_TO_TICKS(200));
            sys_time_ms += 200;
            irq_masked = 0;
            print_time("[Driver] Unmask IRQ again");
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_TX_PERIOD));
        sys_time_ms += WIFI_TX_PERIOD;
    }
}

/* ========================= Firmware Task ========================= */
static void vFirmwareProc(void *p)
{
    uint8_t *buf;
    uint32_t len;
    wifi_pkt_t *pkt;
    wifi_pkt_t *ack;
    TickType_t last_active = xTaskGetTickCount();

    for (;;) {
        /* 🟦 取 TX Ring 資料 */
        xSemaphoreTake(xTxMutex, portMAX_DELAY);
        if (dma_pop(&tx_ring, &buf, &len) == 0) {
            fw_sleep = 0;  // Wake up
            pkt = (wifi_pkt_t *)buf;
            print_time("    [FW] RX pkt from Driver");
            xSemaphoreGive(xTxMutex);

            /* 模擬 Firmware 處理時間 */
            vTaskDelay(pdMS_TO_TICKS(FW_PROC_TIME));
            sys_time_ms += FW_PROC_TIME;

            /* 🟦 產生 ACK 並放入 RX Ring */
            ack = pvPortMalloc(sizeof(wifi_pkt_t));
            ack->seq = pkt->seq;
            snprintf(ack->payload, sizeof(ack->payload), "ACK #%lu", (unsigned long)pkt->seq);

            xSemaphoreTake(xRxMutex, portMAX_DELAY);
            dma_push(&rx_ring, (uint8_t *)ack, sizeof(wifi_pkt_t));
            print_time("    [FW] TX ACK → Driver");
            xSemaphoreGive(xRxMutex);

            vPortFree(pkt);

            /* 模擬中斷通知 Driver */
            if (!irq_masked)
                xSemaphoreGive(xIRQ_Sem);
            else
                print_time("    [FW] IRQ masked, defer IRQ");

            last_active = xTaskGetTickCount();
        } else {
            xSemaphoreGive(xTxMutex);
            if ((xTaskGetTickCount() - last_active) * portTICK_PERIOD_MS > FW_IDLE_TIMEOUT && !fw_sleep) {
                fw_sleep = 1;
                print_time("    [FW] Enter Power Save Mode");
            }
            vTaskDelay(pdMS_TO_TICKS(50));
            sys_time_ms += 50;
        }
    }
}

/* ========================= Bottom Half 處理 ========================= */
static void vDriverBottomHalf(void *p)
{
    uint8_t *buf;
    uint32_t len;
    wifi_pkt_t *pkt;

    for (;;) {
        if (xSemaphoreTake(xIRQ_Sem, portMAX_DELAY) == pdTRUE) {
            irq_masked = 1; // 模擬中斷期間關閉中斷

            /* 🟦 Driver 從 RX Ring 收取 ACK */
            xSemaphoreTake(xRxMutex, portMAX_DELAY);
            if (sdio_read(&buf, &len) == 0) {
                pkt = (wifi_pkt_t *)buf;
                printf("[Driver] RX ← FW : %s\n", pkt->payload);
                vPortFree(pkt);
            }
            xSemaphoreGive(xRxMutex);

            irq_masked = 0;
            print_time("[Driver] IRQ unmasked (ready for next)");
        }
    }
}

/* ========================= main() ========================= */
int main(void)
{
    memset(&tx_ring, 0, sizeof(tx_ring));
    memset(&rx_ring, 0, sizeof(rx_ring));
    xIRQ_Sem = xSemaphoreCreateBinary();
    xTxMutex = xSemaphoreCreateMutex();
    xRxMutex = xSemaphoreCreateMutex();

    print_time("=== FreeRTOS Wi-Fi Driver/Firmware Simulation (Dual Mutex) ===");

    xTaskCreate(vWiFiDriverTx, "DrvTX", 2048, NULL, 2, NULL);
    xTaskCreate(vFirmwareProc, "FW",    2048, NULL, 3, NULL);
    xTaskCreate(vDriverBottomHalf, "DrvBH", 2048, NULL, 1, NULL);

    vTaskStartScheduler();
    for (;;);
    return 0;
}
