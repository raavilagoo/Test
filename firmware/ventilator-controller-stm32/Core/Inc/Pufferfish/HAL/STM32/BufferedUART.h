/*
 * Copyright 2020, the Pez Globo team and the Pufferfish project contributors
 *
 *      Author: Ethan Li
 *
 *  An interrupt-backed UART I/O endpoint, exposing a buffered read/write
 * interface.
 */

#pragma once

#include <Pufferfish/HAL/STM32/Time.h>
#include "Pufferfish/HAL/Interfaces/BufferedUART.h"
#include "Pufferfish/Statuses.h"
#include "Pufferfish/Types.h"
#include "Pufferfish/Util/RingBuffer.h"
#include "stm32h7xx_hal.h"

namespace Pufferfish::HAL::STM32 {

/**
 * UART RX and TX with non-blocking queue interface.
 *
 * Backed by ring buffers serviced by interrupt handlers.
 * This class allows a "polling" approach to reading and writing
 * data which is sent/received asynchronously from the other peer.
 * This provides a comparable interface to Arduino's Serial class,
 * but with a non-blocking interface.
 */
template <AtomicSize rx_buffer_size, AtomicSize tx_buffer_size>
class BufferedUART : public Interfaces::BufferedUART {
 public:
  explicit BufferedUART(UART_HandleTypeDef &huart, Interfaces::Time &time);

  /**
   * Attempt to "pop" the next received byte from the RX queue.
   *
   * Gives up without causing any side-effects if the RX queue is empty;
   * if it gives up, readByte will be left unmodified.
   * @param readByte[[out] the byte popped from the RX queue
   * @return ok on success, empty otherwise
   */
  BufferStatus read(uint8_t &read_byte) volatile override;

  /**
   * Attempt to "push" the provided byte onto the TX queue.
   *
   * Gives up without causing any side-effects if the TX queue is full.
   * @param writeByte the byte to push onto the TX queue
   * @return ok on success, full otherwise
   */
  BufferStatus write(uint8_t write_byte) volatile override;

  /**
   * "Push" bytes in the provided buffer onto the TX queue until either
   * all provided bytes are pushed or the TX queue becomes full.
   *
   * Because this may perform a partial write, you can use the writtenSize
   * output parameter to continue writing a buffer later: just advance the
   * writeBytes buffer pointer by writtenSize, and subtract writtenSize from
   * writeSize in the next method call to attempt to write the remaining bytes.
   * If the TX buffer is full, this will return partial and writtenSize will be
   * 0.
   * @param writeBytes a pointer to the start of the buffer of bytes to push
   * @param writeSize the number of bytes in the buffer to push
   * @param writtenSize[out] the number of bytes successfully pushed onto the TX
   * queue
   * @return ok if all provided bytes were added to the queue, partial otherwise
   */
  BufferStatus write(
      const uint8_t *write_bytes,
      AtomicSize write_size,
      HAL::AtomicSize &written_size) volatile override;

  /**
   * Persistently attempt to "push" the provided byte onto the TX queue
   * until the byte gets pushed or the timeout has elapsed.
   *
   * If the TX queue is initially full, retry up until the timeout,
   * but give up after the timeout.
   * @param writeByte the byte to push onto the TX queue
   * @param uint32_t timeout the length of time in ms to retry pushing the byte
   * if the TX queue is full
   * @return ok on success, full otherwise
   */
  BufferStatus write_block(uint8_t write_byte, uint32_t timeout) volatile override;

  /**
   * Persistently attempt to "push" bytes in the provided buffer onto the TX
   * queue until either all provided bytes are pushed or the timeout has
   * elapsed.
   *
   * Because this may perform a partial write, you can use the writtenSize
   * output parameter to continue writing a buffer later: just advance the
   * writeBytes buffer pointer by writtenSize, and subtract writtenSize from
   * writeSize in the next method call to attempt to write the remaining bytes.
   * If the TX buffer is full, this will return partial and writtenSize will be
   * 0.
   * @param writeBytes a pointer to the start of the buffer of bytes to push
   * @param writeSize the number of bytes in the buffer to push
   * @param timeout the length of time in ms to retry pushing bytes when the TX
   * queue is full
   * @param writtenSize[out] the number of bytes successfully pushed onto the TX
   * queue
   * @return ok if all provided bytes were added to the queue, partial otherwise
   */
  BufferStatus write_block(
      const uint8_t *write_bytes,
      AtomicSize write_size,
      uint32_t timeout,
      HAL::AtomicSize &written_size) volatile override;

  /**
   * Set up the UART interrupt to service the RX queue.
   */
  void setup_irq() volatile;

  /**
   * Handle the UART interrupt which occurs when the RX or TX queue should be
   * serviced.
   */
  void handle_irq() volatile;

  /**
   * A counter of the number of received UART bytes which were discarded.
   *
   * Received UART bytes are discarded when the RX queue is full. If you are
   * seeing many dropped bytes, you are not consuming bytes from the RX queue
   * quickly enough. If you save the returned value from a previous call of this
   * method, you can compare it against the returned value from the latest call
   * of this method to determine how many bytes were dropped since you last
   * called this method.
   * @return the total number of received UART bytes which were discarded.
   */
  [[nodiscard]] uint32_t rx_dropped() const volatile;

 private:
  UART_HandleTypeDef &huart_;
  Interfaces::Time &time_;

  volatile Util::RingBuffer<rx_buffer_size> rx_buffer_;
  volatile Util::RingBuffer<tx_buffer_size> tx_buffer_;

  void handle_irq_rx() volatile;
  void handle_irq_tx() volatile;

  volatile uint32_t rx_dropped_ = 0;
};

static const size_t large_uart_buffer_size = 4096;
using LargeBufferedUART = BufferedUART<large_uart_buffer_size, large_uart_buffer_size>;

static const size_t read_only_uart_buffer_size = 512;
using ReadOnlyBufferedUART = BufferedUART<read_only_uart_buffer_size, 1>;

}  // namespace Pufferfish::HAL::STM32

#include "Pufferfish/HAL/STM32/BufferedUART.tpp"
