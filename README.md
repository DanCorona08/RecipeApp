# Recipe App

The Recipe App is a desktop application that allows users to:
- Add new recipes with ingredients.
- Search for recipes based on ingredients they have.
- View all saved recipes.
- Delete recipes by name.

The app uses SQLite for persistent storage and GTK+ for the graphical user interface. It also includes CSS-based styling and JSON parsing for enhanced functionality.

---

## **Overview**

The Recipe App provides an easy way to manage recipes. Users can add new recipes, search based on available ingredients, and view or delete saved recipes. The app includes a user-friendly graphical interface styled with CSS and supports JSON for extended data handling. It is designed as a lightweight desktop application.

---

## **Dependencies**

The project requires the following libraries:
- GTK+ 3.0 for the graphical user interface.
- SQLite3 for database management.
- libcurl for handling HTTP requests.
- nlohmann-json for JSON parsing.

---

### **Steps**

1. Install the required dependencies:

   ```bash
   sudo apt update
   sudo apt install build-essential libgtk-3-dev sqlite3 libsqlite3-dev libcurl4-openssl-dev nlohmann-json3-dev
   ```

3. Build the project:

   ```bash
   g++ -std=c++17 -o recipe_app main.cpp RecipeManager.cpp `pkg-config --cflags --libs gtk+-3.0` -lsqlite3 -lcurl -ljsoncpp
   ```

4. Run the application:

   ```bash
   ./recipe_app
   ```

---

## **File Structure**

The project includes the following files:

```
.
├── main.cpp              # Entry point for the application, handles the GUI.
├── RecipeManager.cpp     # Core logic for managing recipes (add, delete, search).
├── RecipeManager.h       # Header file for RecipeManager class.
├── styles.css            # CSS file for styling the GTK+ interface.
├── README.txt            # Documentation for the project.
```

---