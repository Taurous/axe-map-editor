#include <vector>
#include <string>

#include "application.hpp"

constexpr int DISPLAY_WIDTH = 1280;
constexpr int DISPLAY_HEIGHT = 720;
constexpr char DISPLAY_TITLE[] = "Simple Map Editor";

int main(int argc, char** argv)
{
	//Aggregate command-line arguments into a std::vector
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i) args.push_back(std::string(argv[i]));

	Application app;

	if (!app.init(args, DISPLAY_TITLE, DISPLAY_WIDTH, DISPLAY_HEIGHT))
	{
		app.quit();
		return -1;
	}

	if (!app.run())
	{
		app.quit();
		return -1;
	}

	//Successful quit
	app.quit();

	return 0;
}