
import net.sf.parapin.ParallelPort;


/**
 * Provides a little support for console-based tests where the test is
 * terminated by a SIGTERM.
 */
public abstract class BaseConsoleTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.1 $";


    protected static ParallelPort  port;


    static {
        // register a shutdown hook to close the parallel port on exit
        Thread  shutdownHook = new Thread("Test shutdown") {
            @Override
            public void run()
            {
                System.out.println("closing " + port);
                if (port != null) {
                    port.close();
                }
            }
        };
        Runtime.getRuntime().addShutdownHook( shutdownHook );

        port = ParallelPort.open();
    }


    protected static void tryToSleep( long msecs )
    {
        try {
            Thread.sleep( msecs );
        }
        catch ( InterruptedException e )
        {
            e.printStackTrace();
        }
    }

}
