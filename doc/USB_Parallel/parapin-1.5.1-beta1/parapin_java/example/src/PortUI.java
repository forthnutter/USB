
import java.awt.BorderLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.WindowConstants;
import javax.swing.table.AbstractTableModel;

import net.sf.parapin.ParallelPort;
import net.sf.parapin.ParallelPort.LogicLevel;
import net.sf.parapin.ParallelPort.Pin;
import net.sf.parapin.ParallelPort.PinRole;


/**
 * Provides a UI to configure the pins, read the logic level at those configured
 * as sensors and to drive the logic level at those configured as drivers.
 */
public final class PortUI
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.3 $";


    private static ParallelPort   port;
    private static PinTableModel  pinTableModel;
    private static RefreshThread  refreshThread;


    public static void main( String[] args )
    {
        port = ParallelPort.open();

        JPanel  portPane = new JPanel( new BorderLayout(5, 5) );
        pinTableModel = new PinTableModel();
        JTable  pinTable = new JTable( pinTableModel );
        pinTable.setDefaultEditor( PinRole.class, new DefaultCellEditor(new JComboBox(PinRole.values())) );
        pinTable.setDefaultEditor( LogicLevel.class, new DefaultCellEditor(new JComboBox(LogicLevel.values())) );
        portPane.add( new JScrollPane(pinTable) );

        JFrame  window = new JFrame("Parallel Port");
        window.setDefaultCloseOperation( WindowConstants.DISPOSE_ON_CLOSE );
        window.getContentPane().add( portPane, BorderLayout.CENTER );
        window.pack();
        window.setVisible( true );
        window.addWindowListener(
            new WindowAdapter()
            {
                // for when the window is closed by the OS
                @Override
                public void windowClosing( WindowEvent ev )
                {
                    refreshThread.shouldRun = false;
                    port.close();
                }
            }
        );

        refreshThread = new RefreshThread();
        refreshThread.start();
    }


    static final class PinTableModel extends AbstractTableModel
    {
        private static final String[] NAME_OF_COLUMN = {
            "pin", "role", "state"
        };

        void refresh()
        {
            fireTableDataChanged();
        }

        public int getColumnCount()
        {
            return NAME_OF_COLUMN.length;
        }

        @Override
        public String getColumnName( int columnIndex )
        {
            return NAME_OF_COLUMN[columnIndex];
        }

        @Override
        public Class<?> getColumnClass( int columnIndex )
        {
            if (0 == columnIndex) { return Integer.class; }
            if (1 == columnIndex) { return ParallelPort.PinRole.class; }
            return ParallelPort.LogicLevel.class;
        }

        public int getRowCount()
        {
            return 17;
        }

        public Object getValueAt( int rowIndex, int columnIndex )
        {
            Pin  pin = port.pin( 1 + rowIndex );
            switch (columnIndex)
            {
            case 0: return new Integer(pin.number());
            case 1: return pin.currentRole();
            case 2: return pin.currentRole() != null ? pin.logicLevel() : null;
            default: throw new IllegalArgumentException();
            }
        }

        @Override
        public boolean isCellEditable( int rowIndex, int columnIndex )
        {
            Pin  pin = port.pin( 1 + rowIndex );

            return 1 == columnIndex  ||
                2 == columnIndex && ParallelPort.PinRole.DRIVER == pin.currentRole();
        }

        @Override
        public void setValueAt( Object editedValue, int rowIndex, int columnIndex )
        {
            Pin  pin = port.pin( 1 + rowIndex );

            if (1 == columnIndex)
            {
                if (editedValue != null)
                {
                    pin.assumeRole( (PinRole) editedValue );
                }
            }
            else {
                pin.driveTo( (LogicLevel) editedValue );
            }
        }

        private static final long  serialVersionUID = 1L;
    }


    private static final class RefreshThread extends Thread
    {
        private volatile boolean  shouldRun = true;

        @Override
        public void run()
        {
            while (shouldRun)
            {
                pinTableModel.refresh();

                try {
                    Thread.sleep( 1000 / 5 );
                }
                catch ( InterruptedException e )
                {
                    e.printStackTrace();
                }
            }
        }
    }

}
