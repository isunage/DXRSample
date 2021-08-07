#include <Win32Application.h>
#include <Test2AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
int main() {
	/*Application*/
	auto application = test::Win32Application::New();
	/*AppDelegate*/
	auto appDelegate = test::Test2AppDelegate::New(640, 480, "Test2AppDelegate");
	/*Delegate*/
	test::Win32Application::Delegate(application, appDelegate);
	application->OnRun();
	return 0;
}