# 🚗 Car Showroom Management System using B+ Tree

A full-featured C program that simulates a car dealership system managing stock, sales, and customer data using **B+ Trees** for efficient searching, sorting, and updates.

---

## 📌 Features

- 📦 **Car Inventory Management** (by VIN)  
  Tracks available and sold cars with full metadata: name, color, price, type, fuel, etc.

- 👨‍💼 **Salesperson Records**  
  Each showroom has a tree of salespersons with data on targets, achievements, and commissions.

- 👤 **Customer Information System**  
  Maintains tree-based records of buyers including loan/EMI details and purchase history.

- 🔎 **Advanced Queries Supported**
  - Merge inventories across showrooms sorted by VIN
  - Track EMI plans by duration
  - Predict sales, identify top salespersons, and search sales by range
  - Display all car details by VIN (sold or unsold)

---

## 🛠 Tech Stack

- **Language**: C  
- **Data Structure**: B+ Tree for fast search, insert, and range queries  
- **File Handling**: To store and retrieve persistent data for each tree

---

## 📦 Compilation & Usage

```bash
gcc Car_Showroom_Management.c -o showroom
./showroom
