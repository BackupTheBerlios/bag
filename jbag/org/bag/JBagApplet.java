package org.bag;

import javax.swing.*;

public class JBagApplet extends JApplet
{
        public void init()
        {
                getContentPane().add(new JLabel("A window should have popped up..."));
                JBagApplication.main(new String[0]);
        }
}
