/* Users tab in BagWin */
package org.bag;

import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.awt.*;
import java.sql.*;
import java.net.*;
import java.io.*;
import java.util.*;

class BWUsers extends JPanel implements ActionListener
{
        private BagWin bagwin;
        private JTable tab;
        private MyTableModel tabmodel;
        
        public BWUsers(BagWin bw)
        {
                super();
                bagwin=bw;
                setLayout(new BorderLayout());
                add(new JLabel("Users:"),BorderLayout.NORTH);
                add(new JScrollPane(tab=new JTable(tabmodel=new MyTableModel())),BorderLayout.CENTER);
                JPanel p=new JPanel();
                JButton b;
                p.add(b=new JButton("New User"));
                p.add(b=new JButton("Modify User"));
                p.add(b=new JButton("Delete User"));
                p.add(b=new JButton("Refresh"));
                b.setActionCommand("refresh");
                b.addActionListener(this);
                add(p,BorderLayout.SOUTH);
                doLayout();
        }
        
        public String getName()
        {
                return "Users";
        }

        public void actionPerformed(ActionEvent event)
        {
                String act=event.getActionCommand();
                if("refresh"==act)refreshUsers();
                else
                System.out.println("Unknown action event: " +act);
        }

        private void refreshUsers()
        {
                try{
                        BagMessage m=(new BagMessage("getusers")).send(bagwin.getSocket());
                        if(m.getReturn()){
                                StringTokenizer st=new StringTokenizer(new String(m.getBlob()),"\n",false);
                                while(st.hasMoreTokens()){
                                        String usr=st.nextToken();
                                        tabmodel.adduser(
                                                usr,
                                                "-",
                                                (new BagMessage("getuseracl")).add(usr).send(bagwin.getSocket()).getArgStr()
                                        );
                                }
                                tabmodel.fireTableDataChanged();
                        }else{
                                JOptionPane.showMessageDialog(this,"Error",
                                        "Could not retrieve users: "+m.getArgStr(),JOptionPane.ERROR_MESSAGE);
                        }
                }catch(Exception e){}
        }

        class MyTableModel extends AbstractTableModel{
                public MyTableModel(){reset();}
                public int getRowCount(){return users.size();}
                public int getColumnCount(){return 3;}
                public Object getValueAt(int r,int c)
                {
                        User u=(User) (users.get(r));
                        switch(c)
                        {
                                case 0:return u.uname;
                                case 1:return u.auth;
                                case 2:return u.acl;
                                default: return "";
                        }
                }
                public String getColumnName(int c)
                {
                        switch(c)
                        {
                                case 0:return "Username";
                                case 1:return "Authentication";
                                case 2:return "global ACL";
                                default: return "";
                        }
                }

                public void reset()
                {
                        users=new Vector();
                }

                public void adduser(String uname,String auth,String acl)
                {
                        users.add(new User(uname,auth,acl));
                }

                private Vector users;

                private class User{
                        public String uname,auth,acl;
                        public User(String un, String au,String ac){uname=un;auth=au;acl=ac;}
                        public boolean equal(Object o){return o.toString().equals(uname);}
                        public String toString(){return uname;}
                }
                
        }
}