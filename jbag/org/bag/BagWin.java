/*Main window of an open connection */

package org.bag;

import javax.swing.*;
import java.awt.event.*;
import java.sql.*;
import java.net.*;
import java.io.*;

class BagWin extends JFrame implements WindowListener,ActionListener
{
        /** listing of allowable connection types (currently ignored)*/
        public static final String[] CONNTYPES=
                {"insecure net","secure net"};

        private Socket sock;
        private BagSocket bsock;

        private String hello;

        private JTabbedPane tab;

        /** Tries to open a connection to the Bag Server. Throws BagWin.OpenException
            on error. Opens a connection Window on success.
            @see BagWin.OpenException
        */
        public BagWin(String conntype,String host,int port,String user,String passwd)
                throws OpenException
        {
                addWindowListener(this);
                try{
                        if(port<=0){
                                dispose();
                                throw new OpenException("Port must be >0");
                        }
                        sock=new Socket(host,port);
                        bsock=new BagSocket(sock);

                        //consume hello message, check for vhost
                        hello=bsock.readLine();
                        if(hello.substring(3,8).equals("vhost")){
                                System.out.println("using virtual host "+host);
                                bsock.write(("0 vhost "+host+"\n").getBytes());
                                hello=bsock.readLine();
                        }
                        setTitle(hello.substring(3));
                        
                        BagMessage m=new BagMessage("auth").add(user).add(passwd).send(bsock);
                        if(!m.getReturn()){
                                throw new OpenException("Unable to authenticate to host "+host);
                        }
                }catch(UnknownHostException e){
                        dispose();
                        throw new OpenException("Unable to connect to host "+host+": "+e.getMessage());
                }catch(IOException e){
                        dispose();
                        throw new OpenException("IO-Error: "+e.getMessage());
                }catch(BagMessage.CommException e){
                        dispose();
                        throw new OpenException("Communication Error: "+e.getMessage());
                }
                //////
                setSize(800,600);

                //////
                JMenuBar mb=new JMenuBar();
                JMenu m;
                JMenuItem mi;
                setJMenuBar(mb);
                //////
                mb.add(m=new JMenu("Connection"));

                m.add(mi=new JMenuItem("Close Connection",'c'));
                mi.setActionCommand("close");
                mi.addActionListener(this);
                                
                tab=new JTabbedPane();
                getContentPane().add(tab);
                
                JComponent c;
                tab.add(new BWProfile(this,conntype,host,port,user));
                tab.add(new BWUsers(this));
                tab.add(new JLabel("hello"));
                
                show();
        }

        public static class OpenException extends Exception {
                public OpenException(String s){super(s);}
        }


        public BagSocket getSocket(){return bsock;}
        
        /*WindowListener*/
        public void windowActivated(WindowEvent e){}
        public synchronized void windowClosed(WindowEvent e)
        {
                if(bsock!=null)try{bsock.close();}catch(Exception ex){}
                bsock=null;
        }
        public synchronized void windowClosing(WindowEvent e)
        {
                if(bsock!=null)try{bsock.close();}catch(Exception ex){}
                bsock=null;
        }
        public void windowDeactivated(WindowEvent e){}
        public void windowDeiconified(WindowEvent e){}
        public void windowIconified(WindowEvent e){}
        public void windowOpened(WindowEvent e){}

        /*ActionListener*/
        public void actionPerformed(ActionEvent event)
        {
                String act=event.getActionCommand();
                if("close"==act){
                        try{//try to quit normally
                                (new BagMessage("quit")).send(bsock);
                        }catch(Exception ex){}
                        try{//use a new try to be sure it is done:
                                bsock.close();
                        }catch(Exception ex){}
                        bsock=null;//nullify to be sure no one else tries to close it again
                        this.dispose();
                }else

                System.out.println("Unknown action event: " +event.getActionCommand());
        }
}