# Recipe App

The Recipe App is a desktop application that allows users to:
- Add new recipes with ingredients.
- Search for recipes based on ingredients they have.
- View all saved recipes.
- Delete recipes by name.

The app uses SQLite for persistent storage and GTK+ for the graphical user interface.

---

## **Features**
1. **Add Recipes**: Enter a recipe name and its ingredients seperated by commas to save it.
2. **Search Recipes**: Find recipes based on the ingredients you have.
3. **View All Recipes**: See all stored recipes.
4. **Delete Recipes**: Remove recipes by name.

---

## **Installation**

### **Prerequisites**
- A Linux-based system (Debian or similar).
- `gcc` (or `g++`) installed.
- `pkg-config` installed.
- `gtk+3.0` development libraries installed.
- SQLite3 library installed

---

## **Build**

### **Step 1: Clone the Repository**
1. Clone to local machine.
2. In terminal change directory to where you stored it locally.

### **Step 2: Install Dependencies**
1. Install the required libraries

   sudo apt update

   sudo apt install build-essential libgtk-3-dev sqlite3 libsqlite3-dev
   
3. Verify that pkg-config can find the GTK+ libraries

   pkg-config --cflags --libs gtk+-3.0

### **Step 3: Compile the Project**
1. Run the following command in the project directory:

   g++ -std=c++17 -o recipe_app main.cpp RecipeManager.cpp `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

### **Usage**
1. Launch the app from the terminal:

   ./recipe_app
