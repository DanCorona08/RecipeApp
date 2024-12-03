#include <gtk/gtk.h>
#include "RecipeManager.h"
#include <string>
#include <vector>

// Global RecipeManager instance
RecipeManager manager;

// Debug function to print all recipes
void debugPrintAllRecipes() {
    std::vector<std::pair<std::string, std::vector<std::string>>> recipes = manager.listAllRecipes();
    g_print("Recipes in Database:\n");
    for (const auto &[name, ingredients] : recipes) {
        g_print(" - %s: ", name.c_str());
        for (const auto &ingredient : ingredients) {
            g_print("%s, ", ingredient.c_str());
        }
        g_print("\n");
    }
}

void on_view_recipes_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *label = GTK_WIDGET(data);

    // Get all recipes from the manager
    auto recipes = manager.listAllRecipes();

    // Format recipes for display
    std::string recipeList;
    for (const auto &[name, ingredients] : recipes) {
        recipeList += name + ": ";
        for (const auto &ingredient : ingredients) {
            recipeList += ingredient + ", ";
        }
        if (!ingredients.empty()) {
            recipeList.pop_back(); // Remove trailing space
            recipeList.pop_back(); // Remove trailing comma
        }
        recipeList += "\n";
    }

    if (recipeList.empty()) {
        recipeList = "No recipes available.";
    }

    gtk_label_set_text(GTK_LABEL(label), recipeList.c_str());
}

void on_delete_recipe_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;

    // Extract widgets
    GtkWidget *entry = widgets[0];
    GtkWidget *statusLabel = widgets[1];

    const char *recipeName = gtk_entry_get_text(GTK_ENTRY(entry));

    // Validate input
    if (strlen(recipeName) == 0) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Please enter a recipe name.");
        return;
    }

    // Attempt to delete the recipe
    if (manager.deleteRecipe(recipeName)) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Recipe deleted successfully.");
    } else {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Failed to delete recipe. Recipe may not exist.");
    }
}

// Callback function for the "Find Recipes" button
void on_find_recipes_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;

    // Extract widgets from the array
    GtkWidget *entry = widgets[0];
    GtkWidget *label = widgets[1];

    // Debug: Print received widget pointers
    g_print("Received Widget Pointers for Find Recipes:\n");
    g_print(" - Entry: %p\n", (void *)entry);
    g_print(" - Label: %p\n", (void *)label);

    // Validate widget types
    if (!GTK_IS_ENTRY(entry)) {
        gtk_label_set_text(GTK_LABEL(label), "Error: Entry widget is invalid or missing.");
        return;
    }

    if (!GTK_IS_LABEL(label)) {
        g_print("Error: Label widget is invalid or missing.\n");
        return;
    }

    // Get user input from the entry
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
    if (text == NULL || strlen(text) == 0) {
        gtk_label_set_text(GTK_LABEL(label), "Please enter some ingredients.");
        return;
    }

    std::string ingredients = text;

    // Parse ingredients into a vector
    auto trim = [](const std::string &str) {
        size_t start = str.find_first_not_of(" \t");
        size_t end = str.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    };

    std::vector<std::string> userIngredients;
    size_t pos = 0;
    while ((pos = ingredients.find(',')) != std::string::npos) {
        userIngredients.push_back(trim(ingredients.substr(0, pos)));
        ingredients.erase(0, pos + 1);
    }
    userIngredients.push_back(trim(ingredients));

    // Debug: Print parsed ingredients
    g_print("Parsed User Ingredients:\n");
    for (const auto &ingredient : userIngredients) {
        g_print(" - %s\n", ingredient.c_str());
    }

    // Find matching recipes
    auto recipes = manager.findRecipes(userIngredients);

    // Format and display results
    if (recipes.empty()) {
        gtk_label_set_text(GTK_LABEL(label), "No matching recipes found.");
    } else {
        std::string result;
        for (const auto &recipe : recipes) {
            result += recipe + "\n";
        }
        gtk_label_set_text(GTK_LABEL(label), result.c_str());
    }
}

// Callback function for the "Add Recipe" button
void on_add_recipe_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;

    // Extract widgets from the array
    GtkWidget *nameEntry = widgets[0];
    GtkWidget *ingredientsEntry = widgets[1];
    GtkWidget *statusLabel = widgets[2];

    // Debug: Print received widget pointers
    g_print("Received Widget Pointers for Add Recipe:\n");
    g_print(" - Name Entry: %p\n", (void *)nameEntry);
    g_print(" - Ingredients Entry: %p\n", (void *)ingredientsEntry);
    g_print(" - Status Label: %p\n", (void *)statusLabel);

    // Validate widget types
    g_return_if_fail(GTK_IS_ENTRY(nameEntry));
    g_return_if_fail(GTK_IS_ENTRY(ingredientsEntry));
    g_return_if_fail(GTK_IS_LABEL(statusLabel));

    // Get user input from the entry fields
    const char *name = gtk_entry_get_text(GTK_ENTRY(nameEntry));
    const char *ingredients = gtk_entry_get_text(GTK_ENTRY(ingredientsEntry));

    // Validate inputs
    if (strlen(name) == 0 || strlen(ingredients) == 0) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Please enter both a name and ingredients.");
        return;
    }

    // Parse ingredients into a vector
    auto trim = [](const std::string &str) {
        size_t start = str.find_first_not_of(" \t");
        size_t end = str.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    };

    std::vector<std::string> ingredientList;
    std::string ingredientsStr = ingredients;
    size_t pos = 0;
    while ((pos = ingredientsStr.find(',')) != std::string::npos) {
        ingredientList.push_back(trim(ingredientsStr.substr(0, pos)));
        ingredientsStr.erase(0, pos + 1);
    }
    ingredientList.push_back(trim(ingredientsStr));

    // Debug: Print parsed ingredients
    g_print("Parsed Ingredients for Add Recipe:\n");
    for (const auto &ingredient : ingredientList) {
        g_print(" - %s\n", ingredient.c_str());
    }

    // Add the recipe to the database
    try {
        manager.addRecipe(name, ingredientList);
        gtk_label_set_text(GTK_LABEL(statusLabel), "Recipe added successfully!");
    } catch (const std::exception &e) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Failed to add recipe.");
    }
}

// Main application activation function
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *findEntry, *addNameEntry, *addIngredientsEntry, *findLabel, *addStatusLabel, *findButton, *addButton;

    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Recipe Recommendation App");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Section 1: Find Recipes
    GtkWidget *findLabelTitle = gtk_label_new("Find Recipes:");
    gtk_grid_attach(GTK_GRID(grid), findLabelTitle, 0, 0, 1, 1);

    findEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(findEntry), "Enter ingredients (comma-separated)");
    gtk_grid_attach(GTK_GRID(grid), findEntry, 0, 1, 2, 1);

    findLabel = gtk_label_new("Your results will appear here...");
    gtk_grid_attach(GTK_GRID(grid), findLabel, 0, 2, 3, 1);

    findButton = gtk_button_new_with_label("Find Recipes");
    gtk_grid_attach(GTK_GRID(grid), findButton, 2, 1, 1, 1);

    GtkWidget **findWidgets = new GtkWidget *[2]{findEntry, findLabel};
    g_signal_connect(findButton, "clicked", G_CALLBACK(on_find_recipes_clicked), findWidgets);

    // Debug: Print pointers for Find Recipes widgets
    g_print("Find Recipes Section:\n");
    g_print(" - Entry Widget: %p\n", (void *)findEntry);
    g_print(" - Label Widget: %p\n", (void *)findLabel);

    // Section 2: Add Recipe
    GtkWidget *addLabelTitle = gtk_label_new("Add a Recipe:");
    gtk_grid_attach(GTK_GRID(grid), addLabelTitle, 0, 3, 1, 1);

    addNameEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(addNameEntry), "Recipe Name");
    gtk_grid_attach(GTK_GRID(grid), addNameEntry, 0, 4, 1, 1);

    addIngredientsEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(addIngredientsEntry), "Ingredients (comma-separated)");
    gtk_grid_attach(GTK_GRID(grid), addIngredientsEntry, 1, 4, 1, 1);

    addButton = gtk_button_new_with_label("Add Recipe");
    gtk_grid_attach(GTK_GRID(grid), addButton, 2, 4, 1, 1);

    addStatusLabel = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), addStatusLabel, 0, 5, 3, 1);

    GtkWidget **addWidgets = new GtkWidget *[3]{addNameEntry, addIngredientsEntry, addStatusLabel};
    g_signal_connect(addButton, "clicked", G_CALLBACK(on_add_recipe_clicked), addWidgets);

    // Section 3: View Recipes
    GtkWidget *viewLabelTitle = gtk_label_new("View All Recipes:");
    gtk_grid_attach(GTK_GRID(grid), viewLabelTitle, 0, 6, 1, 1);

    GtkWidget *viewRecipesLabel = gtk_label_new("Recipes will appear here...");
    gtk_grid_attach(GTK_GRID(grid), viewRecipesLabel, 0, 7, 3, 1);

    GtkWidget *viewRecipesButton = gtk_button_new_with_label("View Recipes");
    gtk_grid_attach(GTK_GRID(grid), viewRecipesButton, 2, 6, 1, 1);

    g_signal_connect(viewRecipesButton, "clicked", G_CALLBACK(on_view_recipes_clicked), viewRecipesLabel);

    // Section 4: Delete Recipe
    GtkWidget *deleteLabelTitle = gtk_label_new("Delete a Recipe:");
    gtk_grid_attach(GTK_GRID(grid), deleteLabelTitle, 0, 8, 1, 1);

    GtkWidget *deleteEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(deleteEntry), "Recipe Name");
    gtk_grid_attach(GTK_GRID(grid), deleteEntry, 0, 9, 2, 1);

    GtkWidget *deleteStatusLabel = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), deleteStatusLabel, 0, 10, 3, 1);

    GtkWidget *deleteButton = gtk_button_new_with_label("Delete Recipe");
    gtk_grid_attach(GTK_GRID(grid), deleteButton, 2, 9, 1, 1);

    GtkWidget **deleteWidgets = new GtkWidget *[2]{deleteEntry, deleteStatusLabel};
    g_signal_connect(deleteButton, "clicked", G_CALLBACK(on_delete_recipe_clicked), deleteWidgets);

    // Debug: Print pointers for Add Recipe widgets
    g_print("Add Recipe Section:\n");
    g_print(" - Name Entry Widget: %p\n", (void *)addNameEntry);
    g_print(" - Ingredients Entry Widget: %p\n", (void *)addIngredientsEntry);
    g_print(" - Status Label Widget: %p\n", (void *)addStatusLabel);

    // Print all recipes in the database
    debugPrintAllRecipes();

    // Show all widgets
    gtk_widget_show_all(window);
}

// Main entry point of the application
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Initialize the GTK application
    app = gtk_application_new("com.recipe.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Run the GTK application
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}