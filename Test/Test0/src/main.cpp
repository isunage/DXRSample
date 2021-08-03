#include <Win32Application.h>
#include <Win32AppDelegate.h>
int main() {
	/*Application*/
	auto application = test::Win32Application::New();
	/*AppDelegate*/
	auto appDelegate = test::Win32AppDelegate::New(640,480, "AppDelegate");
	test::Win32Application::Delegate(application, appDelegate);
	application->OnInit();
	application->OnMainLoop();
	application->OnDestroy();
	return 0;
}