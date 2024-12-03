#ifndef RECIPEMANAGER_H
#define RECIPEMANAGER_H

#include <string>
#include <vector>
#include <sqlite3.h>

class RecipeManager {
public:
    RecipeManager();  // Constructor to initialize the database
    ~RecipeManager(); // Destructor to close the database connection

    void addRecipe(const std::string &name, const std::vector<std::string> &ingredients);
    std::vector<std::string> findRecipes(const std::vector<std::string> &userIngredients);
    std::vector<std::pair<std::string, std::vector<std::string>>> listAllRecipes() const;
    bool deleteRecipe(const std::string &name); // New method to delete a recipe

private:
    sqlite3 *db; // SQLite database connection
    std::vector<std::pair<std::string, std::vector<std::string>>> recipes; // In-memory recipe cache

    void loadRecipes(); // Load recipes from the database into memory
};

#endif
