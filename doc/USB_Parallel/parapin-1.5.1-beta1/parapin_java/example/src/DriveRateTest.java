
import net.sf.parapin.ParallelPort.LogicLevel;
import net.sf.parapin.ParallelPort.Pin;
import net.sf.parapin.ParallelPort.PinRole;


/**
 * Test how fast a pin can be driven.
 *
 * platform                     writes/sec
 * 2000 MHz Dothan, Java 6.0    386,000
 * 400 MHz Mendocino, Java 6.0  305,000
 * 600 MHz Samuel 2, Java 6.0   263,000
 */
public final class DriveRateTest extends BaseConsoleTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.2 $";


    public static void main( String[] args )
    {
        Pin  pin = port.pin( 1 );
        pin.assumeRole( PinRole.DRIVER );

        int  timesDriven = 0;
        long  countingFrom = System.currentTimeMillis();
        while (true)
        {
            // drive the logic level at the pin 1000 times
            for (int i = 0; i < 100; i++)
            {
                pin.driveTo( LogicLevel.HIGH );
                pin.driveTo( LogicLevel.LOW );
                pin.driveTo( LogicLevel.HIGH );
                pin.driveTo( LogicLevel.LOW );
                pin.driveTo( LogicLevel.HIGH );
                pin.driveTo( LogicLevel.LOW );
                pin.driveTo( LogicLevel.HIGH );
                pin.driveTo( LogicLevel.LOW );
                pin.driveTo( LogicLevel.HIGH );
                pin.driveTo( LogicLevel.LOW );
            }
            timesDriven ++;
            long  now = System.currentTimeMillis();

            // if at least a second has passed since the status was updated..
            if (countingFrom + 1000 <= now)
            {
                System.out.print( "\rdriven " );
                System.out.print( timesDriven );
                System.out.print( "000 times, test inaccuracy:" );
                System.out.print( now - (countingFrom + 1000) );
                System.out.print( "  " );
                timesDriven = 0;
                countingFrom = now;
            }
        }
    }

}
