#ifndef ARGUMENTLIST_H
#define ARGUMENTLIST_H

#include <QStringList>


/** @short a simple interface for processing command line arguments

  An object which provides a simple interface to the command
  line arguments of a program. The methods
  @ref getSwitch(QString) and @ref getSwitchArg(QString)
  allow you to process <b>and remove</b> the switches and switched
  arguments in the command line, so that the remaining
  entries in the stringlist can be processed as a
  uniform list.
<br>
  It also happens to be derived from
  QStringList, so you can use any of those
  methods on this object too.
<br>
This helps you avoid entirely using the C arrays
in your application.
 
<br />
Usage.:
<br />
<pre>
int main(int argc, char** argv) {
ArgumentList args(argc, argv);
bool verbose = args.getSwitch("-v");
// get all other switches
QString outputfile = args.getSwitchArg("-o", "default.out");
qout << args[0];  // prints the name of the executable
qout << args[1]; // prints the first unswitched argument
someObject.processEveryFile(args);
}
</pre>
 
@author S. Alan Ezust sae@mcs.suffolk.edu
@since qt 3.2.1

*/

class ArgumentList : public QStringList {
  public:
    /**
    retrieve argument list from the qApp->argc() and argv() methods.
    Only works if a @ref QApplication(argc, argv) was already created.
    */
    ArgumentList();

    /**
     @param argc number of arguments
     @param argv an array of command line arguments, as supplied
       to main().
       @see argsToStringList()
     */
     
    ArgumentList(int argc, char* argv[]) {
        argsToStringlist(argc, argv);
    }

    ArgumentList(const QStringList& argumentList): 
       QStringList(argumentList) {}


    /**
      finds <b>and removes</b> a switch from the string list, if it exists.
      @param option the switch to search for
      @return true if the switch was found
      */
    bool getSwitch(QString option);

    /**
    finds/removes a switch <b>and its accompanying argument</b>
    from the string list, if the switch is found. Does nothing if the
    switch is not found.
    @param option the switch to search for
    @param defaultReturnValue the return value if option is not found in the stringlist
    @return the argument following option, or defaultValue if the option
    is not found.
    */
    QString getSwitchArg(QString option, 
                         QString defaultRetVal=QString());
  private:
    /**
    (Re)loads argument lists into this object. This function is private because
    it is part of the implementation of the class and not intended to be used by
    client code.
    */
    void argsToStringlist(int argc,  char* argv[]);
};
#endif
