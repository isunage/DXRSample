#include <Win32Application.h>
#include <Test0AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
int main() {
	/*Application*/
	auto application = test::Win32Application::New();
	/*AppDelegate*/
	auto appDelegate = test::Test0AppDelegate::New(640,480, "AppDelegate");
	/*Delegate*/
	test::Win32Application::Delegate(application, appDelegate);
	application->OnRun();
	return 0;
}