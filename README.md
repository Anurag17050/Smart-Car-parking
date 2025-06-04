# 🚗 Smart Car Parking System

A C-based smart parking management system using **B-Trees**, **Structures**, and **Dynamic Memory Allocation** to handle vehicle entries, exits, parking allocation, and reporting with persistent storage.

## 🔧 Features

- Efficiently manages up to 50 parking spaces and 100 vehicles.
- Implements **B-Trees** to store and search:
  - Parking space information (status, revenue, frequency)
  - Vehicle information (arrival, exit, charges, membership)
- Persistent data storage via `vehicle.txt` and `parking.txt` using file handling.
- Membership-based allocation:
  - **GOLDEN**: Spaces 1–10
  - **SILVER**: Spaces 11–20
  - **Regular**: Spaces 21–50
- Calculates charges and upgrades membership based on usage:
  - >200 hrs → GOLDEN
  - >100 hrs → SILVER
- Calculates parking charges with discounts:
  - Standard: ₹100 + ₹50/hr (after 3 hrs)
  - Members: 10% discount
- Custom reporting:
  - Sorted views by total amount paid or frequency
  - Range-based amount queries

## 📁 File Structure

- `main.c` – Entire logic including vehicle and parking management
- `vehicle.txt` – Persistent store for vehicle data
- `parking.txt` – Persistent store for parking slot data

### Input Format

Vehicle entry and exit functions use structured inputs such as:

- Vehicle Entry:
  - Vehicle Number
  - Owner Name
  - Entry DateTime (YYYY-MM-DD_HH:MM)

- Vehicle Exit:
  - Vehicle ID
  - Parking Space ID
  - Exit DateTime (YYYY-MM-DD_HH:MM)

## 📊 Sorting and Reporting

The system uses custom B-Trees for:
- `amount_paid` sorting
- `parking_count` frequency tracking
- In-order traversal prints sorted results

## 📌 Tech Stack

- **Language**: C
- **Data Structures**: B-Trees, Structures
- **Concepts Used**:
  - File Handling
  - Dynamic Memory Management
  - Date-Time Parsing (`time.h`)
  - Membership Systems

## 🧠 Future Enhancements

- GUI for real-time monitoring
- REST API layer for remote access
- SQLite integration for scalability
