#include <Win32Application.h>
#include <Test1AppDelegate.h>
#include <RTLib/DX12Context.h>
#include <RTLib/DX12SwapChain.h>
int main() {
	/*Application*/
	auto application = test::Win32Application::New();
	/*AppDelegate*/
	auto appDelegate = test::Test1AppDelegate::New(640, 480, "Test1AppDelegate");
	/*Delegate*/
	test::Win32Application::Delegate(application, appDelegate);
	application->OnRun();
	return 0;
}