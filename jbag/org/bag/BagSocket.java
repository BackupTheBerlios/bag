package org.bag;

import java.net.*;
import java.io.*;
import java.lang.*;
import java.util.*;

class BagSocket
{
        private Socket sock;
        private BufferedInputStream in;
        private OutputStream out;

        public BagSocket(Socket s)throws IOException
        {
                sock=s;
                in=new BufferedInputStream(s.getInputStream());
                out=s.getOutputStream();
        }

        public synchronized void finalize()
        {
                try{
                        close();
                }catch(Exception e){}
        }

        public synchronized void close()throws IOException
        {
                sock.close();
        }

        public synchronized void read(byte[]b)throws IOException
        {
                in.read(b);
        }

        public synchronized String readLine()throws IOException
        {
                String ln=new String();
                do{
                        int b=in.read();
                        if(b<0||((char)b=='\n'))return ln;
                        ln+=(char)b;
                }while(true);
                
        }

        public synchronized void write(byte[]b)throws IOException
        {
                out.write(b);
        }
}