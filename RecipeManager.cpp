#include "RecipeManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Helper Function: Convert to Lowercase
std::string toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Helper Function: Trim Whitespace
std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Constructor: Initialize the SQLite Database
RecipeManager::RecipeManager() {
    if (sqlite3_open("recipes.db", &db)) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        return;
    }

    // Create the recipes table if it doesn't exist
    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS recipes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            ingredients TEXT NOT NULL,
            category TEXT NOT NULL,
            instructions TEXT NOT NULL DEFAULT '',
            favorite INTEGER DEFAULT 0
        );
    )";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Destructor: Close the SQLite Database
RecipeManager::~RecipeManager() {
    if (db) {
        sqlite3_close(db);
    }
}

// Add a Recipe
bool RecipeManager::addRecipe(const std::string &name, const std::vector<std::string> &ingredients, const std::string &category, const std::string &instructions) {
    std::ostringstream oss;
    for (size_t i = 0; i < ingredients.size(); ++i) {
        oss << ingredients[i];
        if (i < ingredients.size() - 1) {
            oss << ",";
        }
    }
    std::string ingredientsStr = oss.str();

    const char *insertSQL = "INSERT INTO recipes (name, ingredients, category, instructions) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, ingredientsStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, category.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, instructions.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            std::cerr << "Failed to add recipe: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
    }
    return false;
}

// API Integration: Helper function for HTTP requests
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// API Integration: Search recipes by ingredient
std::vector<Recipe> RecipeManager::searchByIngredient(const std::string &ingredient) {
    std::vector<Recipe> recipes;
    std::string readBuffer;

    CURL *curl = curl_easy_init();
    if (curl) {
        std::string url = "https://www.themealdb.com/api/json/v1/1/filter.php?i=" + ingredient;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    // Parse JSON response
    nlohmann::json jsonData = nlohmann::json::parse(readBuffer, nullptr, false);
    if (!jsonData.is_discarded() && jsonData["meals"].is_array()) {
        for (const auto &meal : jsonData["meals"]) {
            Recipe recipe;
            recipe.id = std::stoi(meal["idMeal"].get<std::string>());
            recipe.name = meal["strMeal"].get<std::string>();
            recipes.push_back(recipe);
        }
    }

    return recipes;
}

// API Integration: Fetch recipe instructions by ID
std::string RecipeManager::getRecipeInstructions(int recipeID) {
    std::string readBuffer;

    CURL *curl = curl_easy_init();
    if (curl) {
        std::string url = "https://www.themealdb.com/api/json/v1/1/lookup.php?i=" + std::to_string(recipeID);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    // Parse JSON response
    nlohmann::json jsonData = nlohmann::json::parse(readBuffer, nullptr, false);
    if (!jsonData.is_discarded() && jsonData["meals"].is_array()) {
        return jsonData["meals"][0]["strInstructions"].get<std::string>();
    }

    return "No instructions found.";
}

// List All Recipes
std::vector<Recipe> RecipeManager::listAllRecipes() const {
    std::vector<Recipe> recipes;

    const char *selectSQL = "SELECT name, ingredients, category, instructions, favorite FROM recipes;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Recipe recipe;
            recipe.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string ingredientsStr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            recipe.category = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            recipe.instructions = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            recipe.isFavorite = sqlite3_column_int(stmt, 3);

            // Parse ingredients into a vector
            std::istringstream iss(ingredientsStr);
            std::string ingredient;
            while (std::getline(iss, ingredient, ',')) {
                recipe.ingredients.push_back(trim(ingredient));
            }

            recipes.push_back(recipe);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to retrieve recipes: " << sqlite3_errmsg(db) << std::endl;
    }

    return recipes;
}

// Toggle Recipe as Favorite
bool RecipeManager::toggleFavorite(const std::string &name) {
    const char *updateSQL = R"(
        UPDATE recipes
        SET favorite = NOT favorite
        WHERE name = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, updateSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            std::cerr << "Failed to update favorite status: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << std::endl;
    }
    return false;
}

// List Favorite Recipes
std::string RecipeManager::listFavoriteRecipes() const {
    std::string favoriteList;
    const char *selectSQL = "SELECT name, category FROM recipes WHERE favorite = 1;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string category = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            favoriteList += name + " (" + category + ")\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to retrieve favorite recipes: " << sqlite3_errmsg(db) << std::endl;
    }

    return favoriteList;
}

// Filter Recipes by Category
std::string RecipeManager::filterRecipesByCategory(const std::string &category) const {
    std::string filteredList;
    const char *selectSQL = "SELECT name, ingredients FROM recipes WHERE category = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, category.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            std::string ingredients = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            filteredList += name + ": " + ingredients + "\n";
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Failed to filter recipes: " << sqlite3_errmsg(db) << std::endl;
    }

    return filteredList;
}

// Export Recipes to JSON
bool RecipeManager::exportRecipes(const std::string &filePath) const {
    std::ofstream outFile(filePath);
    if (!outFile) {
        std::cerr << "Failed to open file for export." << std::endl;
        return false;
    }

    nlohmann::json jsonExport;

    for (const auto &recipe : listAllRecipes()) {
        nlohmann::json recipeJson;
        recipeJson["name"] = recipe.name;
        recipeJson["ingredients"] = recipe.ingredients;
        recipeJson["category"] = recipe.category;
        recipeJson["instructions"] = recipe.instructions;
        recipeJson["favorite"] = recipe.isFavorite;

        jsonExport.push_back(recipeJson);
    }

    outFile << jsonExport.dump(4);
    outFile.close();
    return true;
}

// Import Recipes from JSON
bool RecipeManager::importRecipes(const std::string &filePath) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Failed to open file for import." << std::endl;
        return false;
    }

    nlohmann::json jsonImport;
    inFile >> jsonImport;

    for (const auto &recipeJson : jsonImport) {
        std::string name = recipeJson["name"];
        std::vector<std::string> ingredients = recipeJson["ingredients"];
        std::string category = recipeJson["category"];
        std::string instructions = recipeJson["instructions"];
        bool isFavorite = recipeJson["favorite"];

        addRecipe(name, ingredients, category, instructions);
        if (isFavorite) {
            toggleFavorite(name);
        }
    }

    return true;
}

// Clear Database
void RecipeManager::clearDatabase() {
    const char *deleteSQL = "DELETE FROM recipes;";
    char *errMsg = nullptr;
    if (sqlite3_exec(db, deleteSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to clear database: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}
