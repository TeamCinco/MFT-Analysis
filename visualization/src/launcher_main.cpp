#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "StockVisualizer.h"
#include "CointegrationVisualizer.h"

enum class VisualizationMode {
    LAUNCHER,
    STOCK_ANALYSIS,
    PAIRS_ANALYSIS
};

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    GLFWwindow* window = glfwCreateWindow(1600, 1000, "MFT Analysis Suite", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if (gl3wInit() != 0) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Set up ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    
    // Enhanced colors for better appearance
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Initialize visualizers
    StockVisualizer stockVisualizer;
    CointegrationVisualizer pairsVisualizer;
    
    // Auto-load sample data for pairs visualizer
    pairsVisualizer.loadCSVFile("cointegration_sample.csv");
    
    VisualizationMode currentMode = VisualizationMode::LAUNCHER;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Analysis")) {
                if (ImGui::MenuItem("Home", nullptr, currentMode == VisualizationMode::LAUNCHER)) {
                    currentMode = VisualizationMode::LAUNCHER;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Stock Analysis", nullptr, currentMode == VisualizationMode::STOCK_ANALYSIS)) {
                    currentMode = VisualizationMode::STOCK_ANALYSIS;
                }
                if (ImGui::MenuItem("Pairs Analysis", nullptr, currentMode == VisualizationMode::PAIRS_ANALYSIS)) {
                    currentMode = VisualizationMode::PAIRS_ANALYSIS;
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // Could show about dialog
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Render based on current mode
        switch (currentMode) {
            case VisualizationMode::LAUNCHER:
                {
                    // Launcher screen
                    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 20), ImGuiCond_Always);
                    
                    ImGui::Begin("MFT Analysis Suite", nullptr, 
                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
                    
                    // Center the content
                    float windowWidth = ImGui::GetWindowSize().x;
                    float windowHeight = ImGui::GetWindowSize().y;
                    
                    ImGui::SetCursorPos(ImVec2(windowWidth * 0.5f - 300, windowHeight * 0.3f));
                    
                    ImGui::BeginGroup();
                    
                    // Title
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font but larger
                    ImGui::Text("MFT Analysis Suite");
                    ImGui::PopFont();
                    
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::Text("Choose your analysis type:");
                    ImGui::Spacing();
                    
                    // Stock Analysis Button
                    if (ImGui::Button("Stock Analysis", ImVec2(250, 60))) {
                        currentMode = VisualizationMode::STOCK_ANALYSIS;
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text("Individual Stock Analysis");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Technical indicators");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Price charts and patterns");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Volume analysis");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• 50+ technical features");
                    ImGui::EndGroup();
                    
                    ImGui::Spacing();
                    ImGui::Spacing();
                    
                    // Pairs Analysis Button
                    if (ImGui::Button("Pairs Analysis", ImVec2(250, 60))) {
                        currentMode = VisualizationMode::PAIRS_ANALYSIS;
                    }
                    ImGui::SameLine();
                    ImGui::BeginGroup();
                    ImGui::Text("Cointegration Pairs Trading");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Statistical arbitrage");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Mean reversion analysis");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Risk-return optimization");
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "• Outlier detection");
                    ImGui::EndGroup();
                    
                    ImGui::Spacing();
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Data Status:");
                    ImGui::Text("Stock Data: %s", stockVisualizer.isDataLoaded() ? "Ready" : "Click 'Load Data' in Stock Analysis");
                    ImGui::Text("Pairs Data: %s", pairsVisualizer.isDataLoaded() ? "Ready" : "Loading...");
                    
                    ImGui::EndGroup();
                    ImGui::End();
                }
                break;
                
            case VisualizationMode::STOCK_ANALYSIS:
                stockVisualizer.renderUI();
                break;
                
            case VisualizationMode::PAIRS_ANALYSIS:
                pairsVisualizer.renderUI();
                break;
        }
        
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.05f, 0.05f, 0.05f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
