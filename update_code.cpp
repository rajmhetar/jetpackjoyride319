#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main() {
    std::ifstream inFile("ECE319K_Lab9H/Lab9HMain.cpp");
    std::vector<std::string> lines;
    std::string line;
    
    // Read all lines
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    
    // Find the case PLAYING section
    int playingCaseStart = -1;
    int switchStart = -1;
    int caseZeroLine = -1;
    int breakLine = -1;
    
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].find("case PLAYING:") != std::string::npos) {
            playingCaseStart = i;
        } else if (playingCaseStart != -1 && lines[i].find("switch(game.backgroundFrame)") != std::string::npos) {
            switchStart = i;
        } else if (switchStart != -1 && lines[i].find("case 0:") != std::string::npos) {
            caseZeroLine = i;
        } else if (caseZeroLine != -1 && lines[i].find("break;") != std::string::npos && breakLine == -1) {
            breakLine = i;
        }
    }
    
    // If we found all necessary positions
    if (playingCaseStart != -1 && switchStart != -1 && caseZeroLine != -1 && breakLine != -1) {
        // Update position after "Update background position and frame" comment
        for (int i = playingCaseStart; i < switchStart; i++) {
            if (lines[i].find("Update background position and frame") != std::string::npos) {
                lines.insert(lines.begin() + i + 1, "                game.backgroundFrame = (game.backgroundFrame + 1) % 3;  // Cycle through 0, 1, 2");
                // Adjust indexes for the insertion
                switchStart++;
                caseZeroLine++;
                breakLine++;
                break;
            }
        }
        
        // Update the ST7735_DrawBitmap line for case 0
        for (int i = caseZeroLine; i <= breakLine; i++) {
            if (lines[i].find("ST7735_DrawBitmap") != std::string::npos) {
                lines[i] = "                        ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_1, 128, 82);";
                break;
            }
        }
        
        // Add cases 1 and 2 after the first break
        std::vector<std::string> newCases = {
            "                    case 1:",
            "                        ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_2, 128, 82);",
            "                        break;",
            "                    case 2:",
            "                        ST7735_DrawBitmap(game.backgroundX, 121, bg_spaceship_3, 128, 82);",
            "                        break;"
        };
        
        lines.insert(lines.begin() + breakLine + 1, newCases.begin(), newCases.end());
    }
    
    // Write the modified file
    std::ofstream outFile("ECE319K_Lab9H/Lab9HMain.cpp");
    for (const auto& line : lines) {
        outFile << line << std::endl;
    }
    outFile.close();
    
    std::cout << "File updated successfully!" << std::endl;
    return 0;
} 