package org.bag;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.sql.*;


public class BMain extends JScrollPane  implements ActionListener
{
        private static final String COLS[]={"Type","Server","Port","User","Params"};
        //private final JTable table;

        public static BMain main;

        public BMain()
        {
                super(new JTable(new String[0][0],COLS));
                main=this;
        }

        public void actionPerformed(ActionEvent event)
        {
                String act=event.getActionCommand();
                if("exit"==act)doexit();else
                if("debugcon"==act)dodebug();else

                if("about"==act)aboutBox();else

                System.out.println(event.getActionCommand());
        }

        protected void doexit()
        {
                System.exit(0);
        }

        private void aboutBox()
        {
                AboutBox b=new AboutBox();
                b.pack();
                b.show();
        }

        private void dodebug()
        {
                OpenDebug od=new OpenDebug();
                od.show();
        }


        ////////////////////////////////////////////////////////////
        private class AboutBox extends JFrame implements ActionListener
        {
                private static final String ABOUT=
"<html><h1>About JBag</h1><hr><br>"+
"JBag is a grafical user interface for the version control system Bag.<p>"+

"<h2>Authors:</h2><ul>"+
"<li>Konrad Rosenbaum &lt;konrad.rosenbaum@gmx.net&gt<br>Design, Base Implementation"+

"</ul><p><h2>Credits:</h2><ul>"+
"<li>The Bag Project (http://????)"+
"<li>PostgreSQL (http://www.postgresql.org)"+
"<li>Sun Java2 (http://www.java.sun.com)"+
"</ul><hr></html>";
                public AboutBox()
                {
                        Container c=getContentPane();
                        c.setLayout(new BorderLayout());

                        c.add(new JLabel(ABOUT),BorderLayout.CENTER);
                        JPanel pan=new JPanel();
                        c.add(pan,BorderLayout.SOUTH);
                        JButton ok;
                        pan.add(ok=new JButton("Close"));
                        ok.addActionListener(this);
                }

                public void actionPerformed(ActionEvent event)
                {
                        dispose();
                }
        }

        /////////////////////////////////////////////////////////
        private class OpenDebug extends JDialog implements ActionListener
        {
                private final JTextField server,port,database,user;
                private final JPasswordField passwd;
                public OpenDebug()
                {
                        setTitle("Open new Debug Connection");
                        Container c=getContentPane();
                        GridBagLayout g;
                        c.setLayout(g=new GridBagLayout());

                        GridBagConstraints gc=new GridBagConstraints();
                        gc.gridwidth=GridBagConstraints.REMAINDER;
                        c.add(new JLabel("Server:"));
                        c.add(server=new JTextField("localhost",32),gc);

                        c.add(new JLabel("Port:"));
                        c.add(port=new JTextField("5432",32),gc);

                        c.add(new JLabel("Database:"));
                        c.add(database=new JTextField(32),gc);

                        c.add(new JLabel("Username:"));
                        c.add(user=new JTextField(32),gc);

                        c.add(new JLabel("Password:"));
                        c.add(passwd=new JPasswordField(32),gc);

                        c.add(new JLabel("  "),gc);

                        JButton btn;
                        c.add(btn=new JButton("Connect"));
                        btn.setActionCommand("connect");
                        btn.addActionListener(this);
                        c.add(btn=new JButton("Cancel"));
                        btn.setActionCommand("cancel");
                        btn.addActionListener(this);

                        pack();
                }

                public void actionPerformed(ActionEvent event)
                {
                        String act=event.getActionCommand();
                        if("cancel"==act)dispose();else
                        if("connect"==act)doconnect();
                }

                private void doconnect()
                {
                        dispose();
                        String constr="jdbc:postgresql://"+server.getText()+":"+
                                port.getText()+"/"+database.getText();
                        //System.out.println(constr);
                        try{
                                Connection con=DriverManager.getConnection(
                                        constr,user.getText(),passwd.getText());
                                //JOptionPane.showMessageDialog(this,"Connected");
                                DebugWin dw=new DebugWin(con);
                                dw.show();
                                //con.close();
                        }catch(SQLException ex){
                                show();
                                ex.printStackTrace();
                                JOptionPane.showMessageDialog(this,
                                 "Connection to database failed: "+ex.getMessage(),
                                 "Database Error",
                                 JOptionPane.ERROR_MESSAGE);
                        }

                }
        }
}
