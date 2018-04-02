
package net.sf.parapin;


/**
 * <p>Represents a parallel port on the system and provides methods to read and
 * write logic levels from and to its pins.</p>
 *
 * <p><em>WARNING:</em> This class is not thread safe.</p>
 */
public final class ParallelPort
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.5 $";

    private static final PinType    BI_DIRECTIONAL_PIN;
    private static final PinType    INPUT_ONLY_PIN;
    private static final PinType    GROUND_PIN;
    private static final PinType    GANGED_PIN;
    private static final PinType[]  TYPE_OF_PIN;

    static {
        System.loadLibrary("net_sf_parapin_ParallelPort");

        BI_DIRECTIONAL_PIN = new PinType( SenseCapability.CAPABLE_OF_SENSING,
                                          DriveCapability.CAPABLE_OF_DRIVING );

        INPUT_ONLY_PIN = new PinType( SenseCapability.CAPABLE_OF_SENSING,
                                      DriveCapability.INCAPABLE_OF_DRIVING );

        GROUND_PIN = new PinType( SenseCapability.INCAPABLE_OF_SENSING,
                                  DriveCapability.INCAPABLE_OF_DRIVING );

        GANGED_PIN = BI_DIRECTIONAL_PIN;

        TYPE_OF_PIN = new PinType[] { BI_DIRECTIONAL_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      GANGED_PIN,
                                      INPUT_ONLY_PIN,
                                      INPUT_ONLY_PIN,
                                      INPUT_ONLY_PIN,
                                      INPUT_ONLY_PIN,
                                      BI_DIRECTIONAL_PIN,
                                      INPUT_ONLY_PIN,
                                      BI_DIRECTIONAL_PIN,
                                      BI_DIRECTIONAL_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
                                      GROUND_PIN,
        };
    }


    private final int    id;
    private final Pin[]  pins;

    private boolean  open;


    /**
     * Opens the default parallel port.  No feature is yet available to open a
     * specific parallel port.
     *
     * @return  an object representing the default parallel port
     * @throws  RuntimeException  if the parallel port could not be opened
     */
    public static ParallelPort open()
    {
        int  response = open0();
        if (response < 0) {
            throw new RuntimeException("could not open parallel port: " + response);
        }
        return new ParallelPort(response);
    }


    /**
     * Obtains an object that represents the specified pin of the parallel port.
     *
     * @param  number  the number of the pin on the parallel port
     * @return  an object representing the specified pin of the parallel port
     * @throws  IllegalArgumentException  if "number" is not in the range 1..17
     * @throws  IllegalStateException  if this parallel port has been closed
     */
    public Pin pin(int number)
    {
        if ( ! open) throw new IllegalStateException();
        if (number < 1  ||  17 < number) throw new IllegalArgumentException();

        Pin  p;
        if ( pins[number - 1] != null )
        {
            p = pins[number - 1];
        }
        else {
            p = new Pin(number);
            pins[number - 1] = p;
        }
        return p;
    }


    /**
     * Closes this parallel port.
     *
     * @throws  IllegalStateException  if this parallel port has already been
     *                                 closed
     */
    public void close()
    {
        if (open) {
            close0(id);
            open = false;
        }
        else {
            throw new IllegalStateException();
        }
    }


    private ParallelPort(int id)
    {
        this.id = id;
        pins = new Pin[17];
        open = true;
    }


    private static native int open0();
    private static native void configurePin0(int port, int pin, byte direction);
    private static native byte logicLevelAtPin0(int port, int pin);
    private static native void drivePin0(int port, int pin, byte logicLevel);
    private static native void close0(int port);


    // ----------------------------------------------------------- nested types

    /**
     * Represents a logic level in the context of a parallel port.
     */
    public static enum LogicLevel
    {
        /** Indicates a logic level of 0, or GND */
        LOW,
        /** Indicates a logic level of 1, or +5V */
        HIGH,
    }


    /**
     * Represents a role for an individual pin on a parallel port.
     */
    public static enum PinRole
    {
        /** Indicates that the PC will sense the logic level */
        SENSOR,
        /** Indicates that the PC will drive the logic level */
        DRIVER,
    }


    static enum SenseCapability
    {
        CAPABLE_OF_SENSING,
        INCAPABLE_OF_SENSING,
    }


    static enum DriveCapability
    {
        CAPABLE_OF_DRIVING,
        INCAPABLE_OF_DRIVING,
    }


    static final class PinType
    {
        private final boolean[]  isCapableOf;

        PinType(SenseCapability sc, DriveCapability dc)
        {
            isCapableOf = new boolean[] {
                SenseCapability.CAPABLE_OF_SENSING == sc,
                DriveCapability.CAPABLE_OF_DRIVING == dc,
            };
        }

        public boolean canAssumeRole( PinRole r )
        {
            return isCapableOf[r.ordinal()];
        }
    }


    /**
     * Represents a pin on a parallel port.
     */
    public final class Pin
    {
        private int         number;
        private PinRole     currentRole;
        private LogicLevel  drivingToLevel;

        Pin(int number)
        {
            this.number = number;
        }

        /**
         * Determines which pin on the parallel port this object represents.
         *
         * @return  the number of this pin
         */
        public int number()
        {
            return number;
        }

        /**
         * Determines if this pin may assume the given role or not.
         *
         * @return  true if this pin may assume the given role or false if not
         */
        public boolean canAssumeRole(PinRole r)
        {
            return TYPE_OF_PIN[number - 1].canAssumeRole(r);
        }

        /**
         * Determines the current role if this pin.
         *
         * @return  the current role of this pin, or null if the role of this
         *          pin is unknown
         */
        public PinRole currentRole()
        {
            return currentRole;
        }

        /**
         * Configures this pin as either {@link PinRole#SENSOR} or
         * {@link PinRole#DRIVER}.
         * 
         * @param  r  the role that this pin should assume
         * @throws  IllegalStateException  if the port on which this pin is
         *                                 located has been closed
         * @throws  IllegalArgumentException  if this pin is incapable of
         *                                    assuming the given role
         */
        public void assumeRole(PinRole r)
        {
            if (open)
            {
                if ( ! canAssumeRole(r))
                {
                    throw new IllegalArgumentException();
                }

                configurePin0( id, number, (byte) r.ordinal() );
                currentRole = r;
            }
            else {
                throw new IllegalStateException();
            }
        }

        /**
         * Determines the logic level present at this pin or the logic level to
         * which it is being driven if this pin is in the {@link PinRole#DRIVER}
         * role.
         *
         * @return  {@link LogicLevel#HIGH} if 5V is present at this pin or
         *          {@link LogicLevel#LOW} if this pin is grounded
         * @throws  IllegalStateException  if the port on which this pin is
         *                                 located has been closed or if the
         *                                 role of this pin is unknown
         */
        public LogicLevel logicLevel()
        {
            if (open)
            {
                if (PinRole.DRIVER == currentRole)
                {
                    return drivingToLevel;
                }
                else if (PinRole.SENSOR == currentRole)
                {
                    return logicLevelAtPin0(id, number) != 0 ? LogicLevel.HIGH : LogicLevel.LOW;
                }
                else {
                    throw new IllegalStateException("unknown role");
                }
            }

            throw new IllegalStateException("port closed");
        }

        /**
         * Drives this pin to the specified logic level.
         *
         * @param  level  the logic level to drive this pin to
         * @throws  IllegalStateException  if the port on which this pin is
         *                                 located has been closed or if this
         *                                 pin is not configured as a
         *                                 {@link PinRole#DRIVER}
         */
        public void driveTo(LogicLevel level)
        {
            if (open) {
                if (PinRole.DRIVER == currentRole) {

                    drivePin0( id, number, (byte) level.ordinal() );
                    drivingToLevel = level;
                    return;
                }
                throw new IllegalStateException("not configured to drive");
            }
            throw new IllegalStateException("port closed");
        }
    }

}
