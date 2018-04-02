
import net.sf.parapin.ParallelPort;


/**
 * Opens the parallel port then close it immediately then exits.
 */
public final class OpenAndCloseTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.2 $";


    public static void main( String[] args )
    {
        ParallelPort  port = ParallelPort.open();
        port.close();
    }

}
