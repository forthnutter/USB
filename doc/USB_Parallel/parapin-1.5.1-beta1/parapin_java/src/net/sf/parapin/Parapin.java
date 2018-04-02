
package net.sf.parapin;


import java.util.ResourceBundle;


/**
 * Shows the revision of the library on stdout.
 */
public final class Parapin
{
    public static final String  COPYRIGHT = "© 2007 Neil Stockbridge";
    public static final String  LICENSE   = "LGPL";
    public static final String  REVISION  = "$Revision: 1.1 $";


    public static void main( String[] args )
    {
        ResourceBundle  b = ResourceBundle.getBundle( Parapin.class.getName() );
        String  revision = b.getString( "REVISION" );
        System.out.println("<library revision=\"" + revision + "\"/>");
    }

}
