#include <mw/core/common/ChildProcess.h>
#include <mw/core/file/FilePath.h>
#include <thread>

void RunTest(const std::string& tests)
{
	std::string path = fs::current_path().u8string() + "/" + tests + "_Tests";

	printf("Testing %s\n", tests.c_str());
	auto pPtr = ChildProcess::Create(std::vector<std::string>({ path }));

	while (pPtr->IsRunning())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	printf("\n\n");

	if (pPtr->GetExitStatus() != 0)
	{
		exit(pPtr->GetExitStatus());
	}
}

int main(int argc, char* argv[])
{
	RunTest("Crypto");
	RunTest("Models");
	RunTest("Util");

	system("pause");
	return 0;
}