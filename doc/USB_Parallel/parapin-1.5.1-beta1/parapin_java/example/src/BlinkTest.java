
import net.sf.parapin.ParallelPort.LogicLevel;
import net.sf.parapin.ParallelPort.Pin;
import net.sf.parapin.ParallelPort.PinRole;


/**
 * Blinks an LED wired to pin 1.
 */
public final class BlinkTest extends BaseConsoleTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.3 $";


    public static void main( String[] args )
    {
        Pin  pin1 = port.pin( 1 );
        pin1.assumeRole( PinRole.DRIVER );

        while (true) {
            pin1.driveTo( LogicLevel.HIGH );
            tryToSleep( 500 );
            pin1.driveTo( LogicLevel.LOW );
            tryToSleep( 500 );
        }
    }

}
