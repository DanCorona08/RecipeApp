#include <gtk/gtk.h>
#include "RecipeManager.h"
#include <string>
#include <vector>

// Global RecipeManager instance
RecipeManager manager;

// Function to load CSS file
void load_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    // Load the styles.css file
    gtk_css_provider_load_from_path(provider, "styles.css", NULL);
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

// Callback to view all recipes
void on_view_recipes_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *label = GTK_WIDGET(data);

    auto recipes = manager.listAllRecipes(); // Get recipes as std::vector<Recipe>
    std::string recipeList;

    for (const auto &recipe : recipes) { // Iterate through the Recipe structure
        recipeList += recipe.name + " (" + recipe.category + "):\n"; // Include category
        recipeList += "Ingredients: ";
        for (const auto &ingredient : recipe.ingredients) {
            recipeList += ingredient + ", ";
        }
        if (!recipe.ingredients.empty()) {
            recipeList.pop_back(); // Remove trailing space
            recipeList.pop_back(); // Remove trailing comma
        }
        recipeList += "\nInstructions: " + recipe.instructions + "\n\n";
    }

    if (recipeList.empty()) {
        recipeList = "No recipes available.";
    }

    gtk_label_set_text(GTK_LABEL(label), recipeList.c_str());
}

// Callback to add a recipe
void on_add_recipe_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;

    GtkWidget *nameEntry = widgets[0];
    GtkWidget *ingredientsEntry = widgets[1];
    GtkWidget *categoryDropdown = widgets[2];
    GtkWidget *instructionsTextView = widgets[3];
    GtkWidget *statusLabel = widgets[4];

    const char *name = gtk_entry_get_text(GTK_ENTRY(nameEntry));
    const char *ingredients = gtk_entry_get_text(GTK_ENTRY(ingredientsEntry));
    const char *category = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(categoryDropdown));

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(instructionsTextView));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    const char *instructions = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    if (!name || strlen(name) == 0 || !ingredients || strlen(ingredients) == 0 ||
        !category || strlen(category) == 0 || !instructions || strlen(instructions) == 0) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Please fill all fields.");
        return;
    }

    std::vector<std::string> ingredientList;
    std::string ingredientsStr = ingredients;
    size_t pos = 0;
    while ((pos = ingredientsStr.find(',')) != std::string::npos) {
        ingredientList.push_back(ingredientsStr.substr(0, pos));
        ingredientsStr.erase(0, pos + 1);
    }
    ingredientList.push_back(ingredientsStr);

    if (manager.addRecipe(name, ingredientList, category, instructions)) {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Recipe added successfully!");
    } else {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Error adding recipe.");
    }
}

// Callback to clear the database
void on_clear_database_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *statusLabel = GTK_WIDGET(data);

    GtkWidget *dialog = gtk_message_dialog_new(
        NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
        GTK_BUTTONS_YES_NO, "Are you sure you want to clear all recipes?");
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_YES) {
        manager.clearDatabase();
        gtk_label_set_text(GTK_LABEL(statusLabel), "Database cleared successfully.");
    } else {
        gtk_label_set_text(GTK_LABEL(statusLabel), "Clear operation canceled.");
    }
}

// Callback to export recipes
void on_export_recipes_clicked(GtkWidget *widget, gpointer data) {
    if (manager.exportRecipes("recipes_export.json")) {
        g_print("Recipes exported successfully to recipes_export.json\n");
    } else {
        g_print("Failed to export recipes.\n");
    }
}

// Callback to import recipes
void on_import_recipes_clicked(GtkWidget *widget, gpointer data) {
    if (manager.importRecipes("recipes_export.json")) {
        g_print("Recipes imported successfully from recipes_export.json\n");
    } else {
        g_print("Failed to import recipes.\n");
    }
}

// Callback to mark a recipe as favorite
void on_mark_favorite_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(data); // Assume entry is passed as data
    const char *recipeName = gtk_entry_get_text(GTK_ENTRY(entry));

    if (manager.toggleFavorite(recipeName)) {
        g_print("Recipe '%s' marked as favorite.\n", recipeName);
    } else {
        g_print("Failed to mark recipe as favorite.\n");
    }
}

// Callback to view favorite recipes
void on_view_favorite_recipes_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *label = GTK_WIDGET(data); // Assume label is passed as data
    std::string favoriteList = manager.listFavoriteRecipes();

    if (favoriteList.empty()) {
        favoriteList = "No favorite recipes found.";
    }

    gtk_label_set_text(GTK_LABEL(label), favoriteList.c_str());
}

// Callback to search recipes by ingredient
void on_search_by_ingredient_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *ingredientEntry = widgets[0];
    GtkWidget *resultLabel = widgets[1];

    const char *ingredient = gtk_entry_get_text(GTK_ENTRY(ingredientEntry));
    if (!ingredient || strlen(ingredient) == 0) {
        gtk_label_set_text(GTK_LABEL(resultLabel), "Please enter an ingredient.");
        return;
    }

    auto recipes = manager.searchByIngredient(ingredient);
    if (recipes.empty()) {
        gtk_label_set_text(GTK_LABEL(resultLabel), "No recipes found for the given ingredient.");
        return;
    }

    std::string recipeList;
    for (const auto &recipe : recipes) {
        recipeList += "ID: " + std::to_string(recipe.id) + " - " + recipe.name + "\n";
    }

    gtk_label_set_text(GTK_LABEL(resultLabel), recipeList.c_str());
}

// Callback to fetch recipe instructions by ID
void on_get_instructions_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *idEntry = widgets[0];
    GtkWidget *instructionsLabel = widgets[1];

    const char *recipeIDStr = gtk_entry_get_text(GTK_ENTRY(idEntry));
    if (!recipeIDStr || strlen(recipeIDStr) == 0) {
        gtk_label_set_text(GTK_LABEL(instructionsLabel), "Please enter a recipe ID.");
        return;
    }

    int recipeID = std::stoi(recipeIDStr);
    std::string instructions = manager.getRecipeInstructions(recipeID);

    gtk_label_set_text(GTK_LABEL(instructionsLabel), instructions.c_str());
}

// Main application activation function
static void activate(GtkApplication *app, gpointer user_data) {
    // Load CSS at activation
    load_css();

    GtkWidget *window;
    GtkWidget *grid;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Recipe Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Section: Search Recipes by Ingredient
    GtkWidget *ingredientEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(ingredientEntry), "Enter Ingredient");
    gtk_grid_attach(GTK_GRID(grid), ingredientEntry, 0, 0, 1, 1);

    GtkWidget *searchButton = gtk_button_new_with_label("Search Recipes");
    gtk_grid_attach(GTK_GRID(grid), searchButton, 1, 0, 1, 1);

    GtkWidget *resultLabel = gtk_label_new("Results will appear here...");
    gtk_grid_attach(GTK_GRID(grid), resultLabel, 0, 1, 2, 1);

    GtkWidget **searchWidgets = new GtkWidget *[2]{ingredientEntry, resultLabel};
    g_signal_connect(searchButton, "clicked", G_CALLBACK(on_search_by_ingredient_clicked), searchWidgets);

    // Section: Fetch Recipe Instructions by ID
    GtkWidget *idEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(idEntry), "Enter Recipe ID");
    gtk_grid_attach(GTK_GRID(grid), idEntry, 0, 2, 1, 1);

    GtkWidget *instructionsButton = gtk_button_new_with_label("Get Instructions");
    gtk_grid_attach(GTK_GRID(grid), instructionsButton, 1, 2, 1, 1);

    GtkWidget *instructionsLabel = gtk_label_new("Instructions will appear here...");
    gtk_grid_attach(GTK_GRID(grid), instructionsLabel, 0, 3, 2, 1);

    GtkWidget **instructionWidgets = new GtkWidget *[2]{idEntry, instructionsLabel};
    g_signal_connect(instructionsButton, "clicked", G_CALLBACK(on_get_instructions_clicked), instructionWidgets);

    // Section: Add Recipe
    GtkWidget *addNameEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(addNameEntry), "Recipe Name");
    gtk_grid_attach(GTK_GRID(grid), addNameEntry, 0, 4, 1, 1);

    GtkWidget *addIngredientsEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(addIngredientsEntry), "Ingredients (comma-separated)");
    gtk_grid_attach(GTK_GRID(grid), addIngredientsEntry, 1, 4, 1, 1);

    GtkWidget *categoryDropdown = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(categoryDropdown), NULL, "Breakfast");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(categoryDropdown), NULL, "Lunch");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(categoryDropdown), NULL, "Dinner");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(categoryDropdown), NULL, "Dessert");
    gtk_grid_attach(GTK_GRID(grid), categoryDropdown, 2, 4, 1, 1);

    GtkWidget *addInstructionsTextView = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(addInstructionsTextView), GTK_WRAP_WORD);
    gtk_text_view_set_justification(GTK_TEXT_VIEW(addInstructionsTextView), GTK_JUSTIFY_LEFT);
    gtk_grid_attach(GTK_GRID(grid), addInstructionsTextView, 0, 5, 3, 2); // Multi-line instructions spanning rows

    GtkWidget *addStatusLabel = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), addStatusLabel, 0, 7, 3, 1);

    GtkWidget *addButton = gtk_button_new_with_label("Add Recipe");
    gtk_grid_attach(GTK_GRID(grid), addButton, 3, 4, 1, 1); // Ensure button is visible and properly placed
    GtkWidget **addWidgets = new GtkWidget *[5]{addNameEntry, addIngredientsEntry, categoryDropdown, addInstructionsTextView, addStatusLabel};
    g_signal_connect(addButton, "clicked", G_CALLBACK(on_add_recipe_clicked), addWidgets);

    // Section: View All Recipes
    GtkWidget *viewRecipesLabel = gtk_label_new("Recipes will appear here...");
    gtk_grid_attach(GTK_GRID(grid), viewRecipesLabel, 0, 8, 3, 1);

    GtkWidget *viewRecipesButton = gtk_button_new_with_label("View Recipes");
    gtk_grid_attach(GTK_GRID(grid), viewRecipesButton, 3, 8, 1, 1);
    g_signal_connect(viewRecipesButton, "clicked", G_CALLBACK(on_view_recipes_clicked), viewRecipesLabel);

    // Section: Favorite Recipes
    GtkWidget *favoriteEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(favoriteEntry), "Recipe Name");
    gtk_grid_attach(GTK_GRID(grid), favoriteEntry, 0, 9, 1, 1);

    GtkWidget *markFavoriteButton = gtk_button_new_with_label("Mark as Favorite");
    gtk_grid_attach(GTK_GRID(grid), markFavoriteButton, 1, 9, 1, 1);
    g_signal_connect(markFavoriteButton, "clicked", G_CALLBACK(on_mark_favorite_clicked), favoriteEntry);

    GtkWidget *viewFavoritesLabel = gtk_label_new("Favorite Recipes will appear here...");
    gtk_grid_attach(GTK_GRID(grid), viewFavoritesLabel, 0, 10, 3, 1);

    GtkWidget *viewFavoritesButton = gtk_button_new_with_label("View Favorite Recipes");
    gtk_grid_attach(GTK_GRID(grid), viewFavoritesButton, 3, 10, 1, 1);
    g_signal_connect(viewFavoritesButton, "clicked", G_CALLBACK(on_view_favorite_recipes_clicked), viewFavoritesLabel);

    // Section: Export and Import Recipes
    GtkWidget *exportButton = gtk_button_new_with_label("Export Recipes");
    gtk_grid_attach(GTK_GRID(grid), exportButton, 0, 11, 1, 1);
    g_signal_connect(exportButton, "clicked", G_CALLBACK(on_export_recipes_clicked), NULL);

    GtkWidget *importButton = gtk_button_new_with_label("Import Recipes");
    gtk_grid_attach(GTK_GRID(grid), importButton, 1, 11, 1, 1);
    g_signal_connect(importButton, "clicked", G_CALLBACK(on_import_recipes_clicked), NULL);

    // Section: Clear Database
    GtkWidget *clearButton = gtk_button_new_with_label("Clear Database");
    gtk_grid_attach(GTK_GRID(grid), clearButton, 0, 12, 1, 1);
    GtkWidget *clearStatusLabel = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), clearStatusLabel, 1, 12, 3, 1);
    g_signal_connect(clearButton, "clicked", G_CALLBACK(on_clear_database_clicked), clearStatusLabel);

    gtk_widget_show_all(window);
}

// Main entry point
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.recipe.manager", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
