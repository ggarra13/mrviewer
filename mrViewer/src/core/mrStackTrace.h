
BEGIN_NAMESPACE( mr )

class ExceptionHandler
{
   public:
     ExceptionHandler();
     ~ExceptionHandler();
     
     static void ShowStack();
   private:
     static void demangle( const char* name );
     static void bt_sighandler(int sig, siginfo_t *info,
			       void *secret);
     void install_signal_handler();
     void restore_signal_handler();

     struct sigaction oldSIGSEGV;
     struct sigaction oldSIGUSR1;
     struct sigaction oldSIGBUS;
     struct sigaction oldSIGILL;
     struct sigaction oldSIGFPE;
};

END_NAMESPACE( mr )

extern mr::ExceptionHandler gExceptionHandler;
