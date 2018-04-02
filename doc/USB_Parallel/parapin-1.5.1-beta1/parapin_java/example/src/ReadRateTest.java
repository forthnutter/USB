
import net.sf.parapin.ParallelPort.Pin;
import net.sf.parapin.ParallelPort.PinRole;


/**
 * Test how fast a pin can be read.
 *
 * platform                     reads/sec
 * 2000 MHz Dothan, Java 6.0    686,000
 * 400 MHz Mendocino, Java 6.0  465,000
 * 600 MHz Samuel 2, Java 6.0   391,000
 */
public final class ReadRateTest extends BaseConsoleTest
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.2 $";


    public static void main( String[] args )
    {
        Pin  pin = port.pin( 1 );
        pin.assumeRole( PinRole.SENSOR );

        int  timesRead = 0;
        long  countingFrom = System.currentTimeMillis();
        while (true)
        {
            // read the logic level at the pin 1000 times
            for (int i = 0; i < 100; i++)
            {
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
                pin.logicLevel();
            }
            timesRead ++;
            long  now = System.currentTimeMillis();

            // if at least a second has passed since the status was updated..
            if (countingFrom + 1000 <= now)
            {
                System.out.print( "\rread " );
                System.out.print( timesRead );
                System.out.print( "000 times, test inaccuracy:" );
                System.out.print( now - (countingFrom + 1000) );
                System.out.print( "  " );
                timesRead = 0;
                countingFrom = now;
            }
        }
    }

}
