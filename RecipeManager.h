#ifndef RECIPEMANAGER_H
#define RECIPEMANAGER_H

#include <string>
#include <vector>
#include <sqlite3.h>

// Recipe Structure
struct Recipe {
    int id = 0; // ID for API-based recipes
    std::string name;
    std::vector<std::string> ingredients;
    std::string category;
    std::string instructions;
    bool isFavorite = false;
};

// RecipeManager Class
class RecipeManager {
public:
    RecipeManager();  // Constructor to initialize the database
    ~RecipeManager(); // Destructor to close the database

    // Recipe Management
    bool addRecipe(const std::string &name, const std::vector<std::string> &ingredients, const std::string &category, const std::string &instructions);
    std::vector<Recipe> listAllRecipes() const;
    bool toggleFavorite(const std::string &name);
    std::string listFavoriteRecipes() const;

    // Category and Filtering
    std::string filterRecipesByCategory(const std::string &category) const;

    // Export/Import Recipes
    bool exportRecipes(const std::string &filePath) const;
    bool importRecipes(const std::string &filePath);

    // Database Management
    void clearDatabase();

    // API Integration
    std::vector<Recipe> searchByIngredient(const std::string& ingredient); // Search recipes by ingredient
    std::string getRecipeInstructions(int recipeID); // Fetch instructions by recipe ID

    void displayRecipeUI(const Recipe& recipe);

private:
    sqlite3 *db; // SQLite database connection
};

#endif // RECIPEMANAGER_H
