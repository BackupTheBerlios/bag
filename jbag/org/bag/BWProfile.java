/* Profile tab in BagWin */
package org.bag;

import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import java.sql.*;
import java.net.*;
import java.io.*;

class BWProfile extends JPanel
{
        private BagWin bagwin;

        public BWProfile(BagWin bw,String conntype,String host,int port,String user)
        {
                super();
                bagwin=bw;
                setLayout(new BorderLayout());

                String tab[][]=new String[3][2];
                String cols[]={"x","y"};
                tab[0][0]="Server:";
                tab[0][1]=host+":"+String.valueOf(port);
                tab[1][0]="Protocol:";
                tab[1][1]=conntype;
                tab[2][0]="User:";
                tab[2][1]=user;

                add(new JTable(tab,cols),BorderLayout.CENTER);
        }

        public String getName()
        {
                return "Connection Profile";
        }
}