package org.bag;

import javax.swing.*;
import java.awt.event.*;
import java.sql.*;

class DebugWin extends JFrame
{
        public DebugWin(Connection con)
        {
                try{
                con.close();
                }catch(SQLException e){}
        }
}