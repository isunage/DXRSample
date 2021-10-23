#include <Win32Application.h>
#include <Test4AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
int main() {
	/*Application*/
	auto application = test::Win32Application::New();
	/*AppDelegate*/
	auto appDelegate = test::Test4AppDelegate::New(640, 480, "Test4AppDelegate");
	/*Delegate*/
	test::Win32Application::Delegate(application, appDelegate);
	application->OnRun();
	return 0;
}