
# 🛠️ C Server: IPC and Memory Mapping Utility

A low-level system programming project written in C that implements a dedicated **Server process** for handling Inter-Process Communication (IPC) requests. The server uses **Named Pipes (FIFOs)** for communication and leverages **Shared Memory** and **Memory Mapping** for high-performance data transfer and file processing.

This server is designed to handle commands from a separate client process (not provided) to perform tasks such as echo, shared memory management, and advanced file reading.

-----

## 💻 Technology Stack & Setup

  * **Language:** C (POSIX Standards)
  * **IPC:** Named Pipes (`mkfifo`)
  * **Memory Management:** Shared Memory (`shm_open`, `mmap`), File Mapping (`mmap`)
  * **System Calls:** `open`, `read`, `write`, `lseek`, `ftruncate`, `shm_open`, `mmap`, `munmap`, `unlink`.


## 📡 IPC and Communication Protocol

The server uses a request/response model based on two named pipes and commands delimited by the `!` character.

| Component | Constant | Mode | Purpose |
| :--- | :--- | :--- | :--- |
| **Request Pipe (FIFO)** | `REQ_PIPE_59592` | Read-Only | Server reads commands and parameters from the client. |
| **Response Pipe (FIFO)** | `RESP_PIPE_59592` | Write-Only | Server creates this pipe and writes responses/status back to the client. |
| **Shared Memory** | `/PHguy5D` | Read/Write | Used as a fast, common buffer for transferring data between the server and the client. |

-----

## 🚀 Supported Commands

The server responds to several commands, processing requests read from `REQ_PIPE_59592` and sending results to `RESP_PIPE_59592`. All successful responses include `SUCCESS!`; all failures include `ERROR!`.

| Command | Request Protocol | Server Action |
| :--- | :--- | :--- |
| `BEGIN!` | N/A | Server initial handshake. Responds with `BEGIN!`. |
| `ECHO!` | N/A | Responds with `ECHO!VARIANT!` + 4 bytes of integer `59592`. |
| `CREATE_SHM!` | + 4 bytes (size) | Creates/opens shared memory `/PHguy5D` with the requested size and maps it. |
| `WRITE_TO_SHM!` | + 4 bytes (offset) + 4 bytes (value) | Writes the 4-byte `value` as an unsigned int at the specified `offset` in the shared memory segment. |
| `MAP_FILE!` | + String (file\_name) + `!` | Opens the specified file and maps its entire content into the server's memory space using `mmap`. |
| `READ_FROM_FILE_OFFSET!` | + 4 bytes (offset) + 4 bytes (bytes\_to\_read) | Reads a specified number of bytes from the **mapped file** at the given file offset, copying the data into the **shared memory** buffer (starting at SHM offset 0). |
| `READ_FROM_FILE_SECTION!`| + 4 bytes (section\_nr) + 4 bytes (offset) + 4 bytes (bytes\_to\_read) | **Advanced Parsing:** Interprets the mapped file as a custom binary structure. Locates the data segment of the specified section and copies the requested bytes (starting at the section's internal offset) into the **shared memory** buffer. |
| `EXIT!` | N/A | Breaks the command loop, closes pipes and memory mappings, and unlinks `RESP_PIPE_59592`. |

-----

## 🧹 Cleanup

The server ensures proper resource management:

  * All file descriptors (`fd_read`, `fd_write`, `shared_memory_fd`) are closed.
  * The dynamically created response pipe (`RESP_PIPE_59592`) is removed from the file system using `unlink`.
  * Unmapping the shared memory and the file map (implicitly handled, but good practice would include explicit `munmap` and `shm_unlink`).
