
import net.sf.parapin.ParallelPort;
import net.sf.parapin.ParallelPort.Pin;


/**
 * Continuously prints the logic level of pin 1 to stdout.
 */
public final class InputTest extends BaseConsoleTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.2 $";


    public static void main( String[] args )
    {
        Pin  pin = port.pin( 1 );
        pin.assumeRole( ParallelPort.PinRole.SENSOR );

        while (true) {
            System.out.println( pin.logicLevel() );
            tryToSleep( 1000 );
        }
    }

}
