# Hướng dẫn build và chạy dự án Tetris

Hướng dẫn này dành cho Windows và macOS, sử dụng VS Code hoặc CLion.

## 1. Kiểm tra CMake

Mở Terminal trên macOS hoặc PowerShell trên Windows:

```sh
cmake --version
```

Nếu hiện phiên bản CMake thì chuyển sang bước tiếp theo.

Nếu máy chưa có CMake:

- macOS:

  ```sh
  brew install cmake
  ```

  Nếu không dùng Homebrew, tải CMake tại <https://cmake.org/download/>.

- Windows: tải CMake tại <https://cmake.org/download/>. Khi cài, chọn
  **Add CMake to the system PATH** rồi mở lại PowerShell.

Kiểm tra lại sau khi cài:

```sh
cmake --version
```

## 2. Lấy source code

Clone lần đầu:

```sh
git clone https://github.com/UIT-nhom-0x/SS004-Tetris-0x.git
cd SS004-Tetris-0x
```

Nếu đã clone trước đó, vào thư mục dự án và cập nhật code:

```sh
git switch main
git pull origin main
```

## 3. Chạy bằng VS Code

VS Code cần extension **CMake Tools**. Nếu Command Palette không có các lệnh
`CMake: ...`, mở Extensions, tìm `CMake Tools` và cài extension này.

1. Mở VS Code và chọn **File > Open Folder**.
2. Chọn thư mục `SS004-Tetris-0x` có file `CMakeLists.txt`.
3. Mở Command Palette:
   - macOS: `Cmd + Shift + P`.
   - Windows: `Ctrl + Shift + P`.
4. Chạy **CMake: Select a Kit**:
   - macOS: chọn Clang.
   - Windows: chọn Visual Studio `amd64` hoặc compiler C++ đang sử dụng.
5. Chạy **CMake: Configure**.
6. Chạy **CMake: Build**.

Sau khi build xong, mở Terminal trong VS Code và chạy:

macOS:

```sh
./build/tetris
```

Windows:

```powershell
.\build\Release\tetris.exe
```

Nếu không có thư mục `Release`, thử:

```powershell
.\build\tetris.exe
```

## 4. Chạy bằng CLion

1. Mở CLion và chọn **Open**.
2. Chọn thư mục `SS004-Tetris-0x` có file `CMakeLists.txt`.
3. Chờ CLion tải xong CMake project.
4. Chọn target `tetris` ở góc trên bên phải.
5. Nhấn **Run** để build và chạy game.

Nếu CLion yêu cầu chọn toolchain:

- macOS: chọn Clang.
- Windows: chọn Visual Studio hoặc MinGW đang có trên máy.

## 5. Build bằng Terminal

Có thể dùng cách này nếu không muốn build bằng giao diện IDE.

### macOS

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/tetris
```

### Windows

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\tetris.exe
```

## 6. Lỗi CMake cache

Nếu gặp lỗi `CMakeCache.txt directory is different`, xóa thư mục build cũ rồi
build lại.

macOS:

```sh
rm -rf build
cmake -S . -B build
cmake --build build
```

Windows PowerShell:

```powershell
Remove-Item -Recurse -Force .\build
cmake -S . -B build
cmake --build build --config Release
```

## 7. Điều khiển

| Phím | Thao tác |
| --- | --- |
| `A` hoặc `←` | Sang trái |
| `D` hoặc `→` | Sang phải |
| `S` hoặc `↓` | Đi xuống |
| `W` hoặc `↑` | Xoay khối |
| `R` | Chơi lại |
| `Q` | Thoát game |
