# Capestone-Project
# 🗂️ Linux File Explorer (C++ Console-Based)

### 👨‍💻 Developed by: Satapathy Raj Aryan

---

## 📘 **Project Overview**

The **File Explorer Application** is a **console-based project** developed in **C++**, designed to interface directly with the **Linux Operating System**.  
It allows users to perform and manage a wide range of **file and directory operations** through a clean terminal-based interface using the **ncurses** library.

This project acts as a simplified version of a graphical file manager — providing features like **navigation, file manipulation, searching, and permission management** — all within the command-line environment.

---

## 🎯 **Objective**

The objective of this project is to gain a deeper understanding of:
- Linux **file handling mechanisms**
- **Directory traversal** and navigation
- System-level operations through **C++ and Linux system calls**
- Command-line application design using **ncurses**

---

## ⚙️ **Features**

✅ List all files and directories in the current path  
✅ Navigate between directories (open / back)  
✅ Create new files and folders  
✅ Rename, copy, move, and delete files or directories  
✅ Search for files by name recursively  
✅ Modify file permissions using numeric codes (e.g., `755`, `644`)  
✅ Color-coded terminal interface using ncurses  
✅ Error handling and visual status feedback  

---

## 🧩 **Technologies Used**

- **Language:** C++ (C++17 Standard)
- **Libraries:**
  - `<filesystem>` – for file and directory operations
  - `<ncurses>` – for terminal UI rendering
  - `<fstream>`, `<iomanip>`, `<vector>`, `<string>` – for input/output and data handling
- **Operating System:** Linux

---

## 🧠 **Installation Instructions**

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/<your-username>/Linux-File-Explorer.git
   cd Linux-File-Explorer
2.Install Required Libraries:
sudo apt update
sudo apt install libncurses5-dev libncursesw5-dev
3.Compile the Program:
g++ FileExplorer.cpp -o explorer -lncurses -std=c++17
4.Run the Application:
./explorer
