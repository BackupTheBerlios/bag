package org.bag;

import javax.swing.*;

public class JBagApplication {
    public static void main(String[] args) {
        JFrame frame = new JFrame("JBag");
        BMain main = new BMain();
        frame.getContentPane().add(main);

        JMenuBar mb=new JMenuBar();
        JMenu m;
        JMenuItem mi;
        frame.setJMenuBar(mb);
        //////
        mb.add(m=new JMenu("Connection"));
        
        m.add(mi=new JMenuItem("Open new Connection",'o'));
        mi.setActionCommand("opencon");
        mi.addActionListener(main);

        try {
                Class.forName("org.postgresql.Driver");
                m.add(mi=new JMenuItem("Open Debug Connection",'d'));
                mi.setActionCommand("debugcon");
                mi.addActionListener(main);
        }catch(ClassNotFoundException e){}

/*        java.util.Enumeration e=java.sql.DriverManager.getDrivers();
        try{
                while(true)System.out.println(e.nextElement());
        }catch(java.util.NoSuchElementException ex){}
*/
        
        m.addSeparator();
        m.add(mi=new JMenuItem("Exit",'e'));
        mi.setActionCommand("exit");
        mi.addActionListener(main);

        //////
        mb.add(m=new JMenu("Help"));

        m.add(mi=new JMenuItem("Manual",'m'));
        m.add(mi=new JMenuItem("Index",'i'));
        m.addSeparator();
        m.add(mi=new JMenuItem("About JBag",'a'));
        mi.setActionCommand("about");
        mi.addActionListener(main);

        //////
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.pack();
        frame.setVisible(true);
    }
}
