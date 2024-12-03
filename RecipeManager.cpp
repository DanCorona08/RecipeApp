#include "RecipeManager.h"
#include <gtk/gtk.h>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

// Helper function to convert a string to lowercase
std::string toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Constructor
RecipeManager::RecipeManager() {
    // Open the SQLite database
    if (sqlite3_open("recipes.db", &db)) {
        g_print("Failed to open database: %s\n", sqlite3_errmsg(db));
        db = nullptr;
        return;
    }

    // Create the recipes table if it doesn't exist
    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS recipes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            ingredients TEXT NOT NULL
        );
    )";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        g_print("Failed to create table: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    // Load existing recipes from the database
    loadRecipes();
}

// Destructor
RecipeManager::~RecipeManager() {
    if (db) {
        sqlite3_close(db);
    }
}

bool RecipeManager::deleteRecipe(const std::string &name) {
    // Remove from database
    const char *deleteSQL = "DELETE FROM recipes WHERE name = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            g_print("Failed to delete recipe: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);

        // Remove from in-memory cache
        auto it = std::remove_if(recipes.begin(), recipes.end(),
                                 [&](const auto &recipe) { return recipe.first == name; });

        if (it != recipes.end()) {
            recipes.erase(it, recipes.end());
            g_print("Recipe '%s' deleted successfully.\n", name.c_str());
            return true;
        }
    } else {
        g_print("Failed to prepare delete statement: %s\n", sqlite3_errmsg(db));
    }

    return false;
}

// Load recipes from the database into memory
void RecipeManager::loadRecipes() {
    const char *selectSQL = "SELECT name, ingredients FROM recipes;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string ingredientsStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));

            // Split ingredients string into a vector
            std::vector<std::string> ingredients;
            std::istringstream iss(ingredientsStr);
            std::string ingredient;
            while (std::getline(iss, ingredient, ',')) {
                ingredients.push_back(ingredient);
            }

            recipes.push_back({name, ingredients});
        }
        sqlite3_finalize(stmt);
    } else {
        g_print("Failed to load recipes: %s\n", sqlite3_errmsg(db));
    }
}

// Add a recipe, avoiding duplicates
void RecipeManager::addRecipe(const std::string &name, const std::vector<std::string> &ingredients) {
    // Check for duplicates
    for (const auto &recipe : recipes) {
        if (recipe.first == name) {
            g_print("Recipe '%s' already exists. Skipping.\n", name.c_str());
            return;
        }
    }

    // Convert ingredients to a comma-separated string
    std::ostringstream oss;
    for (size_t i = 0; i < ingredients.size(); ++i) {
        oss << ingredients[i];
        if (i < ingredients.size() - 1) {
            oss << ",";
        }
    }
    std::string ingredientsStr = oss.str();

    // Insert into the database
    const char *insertSQL = "INSERT INTO recipes (name, ingredients) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ingredientsStr.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            g_print("Failed to insert recipe: %s\n", sqlite3_errmsg(db));
        } else {
            // Add to in-memory cache
            recipes.push_back({name, ingredients});
            g_print("Recipe Added: %s\n", name.c_str());
        }

        sqlite3_finalize(stmt);
    } else {
        g_print("Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
    }
}

// Find matching recipes
std::vector<std::string> RecipeManager::findRecipes(const std::vector<std::string> &userIngredients) {
    std::vector<std::string> matches;

    // Convert user ingredients to lowercase
    std::vector<std::string> lowerUserIngredients;
    for (const auto &ingredient : userIngredients) {
        lowerUserIngredients.push_back(toLower(ingredient));
    }

    // Check each recipe
    for (const auto &recipe : recipes) {
        const std::vector<std::string> &recipeIngredients = recipe.second;

        // Debug: Print recipe being checked
        g_print("Checking Recipe: %s\n", recipe.first.c_str());

        bool allIngredientsMatch = std::all_of(recipeIngredients.begin(), recipeIngredients.end(),
            [&](const std::string &recipeIngredient) {
                return std::find(lowerUserIngredients.begin(), lowerUserIngredients.end(),
                                 toLower(recipeIngredient)) != lowerUserIngredients.end();
            });

        if (allIngredientsMatch) {
            g_print(" - Match found: %s\n", recipe.first.c_str());
            matches.push_back(recipe.first);
        } else {
            g_print(" - No match for: %s\n", recipe.first.c_str());
        }
    }

    return matches;
}

// List all recipes
std::vector<std::pair<std::string, std::vector<std::string>>> RecipeManager::listAllRecipes() const {
    return recipes;
}
