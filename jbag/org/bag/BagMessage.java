package org.bag;

import java.net.*;
import java.io.*;
import java.lang.*;
import java.util.*;

class BagMessage
{
        private Vector args;
        private byte[] blob;
        private boolean retok;

        /** communication exception...*/
        public static class CommException extends Exception
        {
                /** the lower layers broke connection*/
                public static final int BROKENPIPE=1;
                /** interpretation failed on the protocol level*/
                public static final int PROTOERROR=2;
                private int _type;
                protected CommException(int type,String msg)
                {
                        super(msg);
                        _type=type;
                }
                /** get the type of  the exception*/
                int type(){return _type;};
        }

        /** create a message object of type arg0*/
        public BagMessage(String arg0)
        {
                args=new Vector();
                add(arg0);
                retok=true;
                blob=null;
        }

        /** internally: initialize a bagmessage by return code*/
        private BagMessage(boolean ret)
        {
                retok=ret;
                args=new Vector();
                blob=null;
        }

        /** add a string to the message*/
        public BagMessage add(String arg)
        {
                args.addElement(arg);
                return this;
        }

        /** send the message on sock and return the servers answer

        TODO: String quotation is yet not understood*/
        public BagMessage send(BagSocket sock)throws CommException
        {
                int bloblen;
                if(blob==null)bloblen=0;
                else bloblen=blob.length;
                String tosend=new String(Integer.toString(bloblen));
                for(Enumeration el=args.elements();el.hasMoreElements();)
                        tosend+=" "+el.nextElement();
                tosend+='\n';
                try{
                        System.out.println("Sending: "+tosend);
                        //send line
                        sock.write(tosend.getBytes());
                        //send blob
                        if(bloblen>0)sock.write(blob);
                        //read line
                        String recv=sock.readLine();
                        System.out.println("Received: "+recv);
                        BagMessage m;
                        //parse it
                        if(recv.length()<2)throw new CommException(CommException.PROTOERROR,"received line is not long enough");
                        if(recv.charAt(0)=='+')m=new BagMessage(true);else
                        if(recv.charAt(0)=='-')m=new BagMessage(false);
                        else throw new CommException(CommException.PROTOERROR,"return string did not begin with + or -");

                        StringTokenizer st=new StringTokenizer(recv.substring(1)," ");
                        if(!st.hasMoreTokens())throw new CommException(CommException.PROTOERROR,"return string contains no blob length");
                        try{
                                bloblen=Integer.decode(st.nextToken()).intValue();
                        }catch(NumberFormatException e){
                                throw new CommException(CommException.PROTOERROR,"bloblen expected but got string");
                        }
                        if(bloblen<0){
                                throw new CommException(CommException.PROTOERROR,"illegal bloblen");
                        }

                        while(st.hasMoreTokens())m.add(st.nextToken());

                        //read blob
                        if(bloblen>0){
                                byte[] b=new byte[bloblen];
                                sock.read(b);
                                m.setBlob(b);
                        }

                        //return message
                        return m;
                
                }catch(IOException e){
                        throw new CommException(CommException.BROKENPIPE,e.getMessage());
                }
         }

        /** used by send */
        private void setReturn(boolean ret)
        {
                retok=ret;
        }

        /** get the return code of the message*/
        public boolean getReturn()
        {
                return retok;
        }

        /** set a blob to be sent*/
        public void setBlob(byte[]b)
        {
                if(b!=null)blob=(byte[])b.clone();
                else blob=null;
        }

        /** get the arguments of the message as String Array*/
        public String[] getArgs()
        {
                String cln[]=new String[args.size()];
                for(int i=0;i<args.size();i++)
                        cln[i]=(String)args.elementAt(i);
                return cln;
        }

        /** get the arguments as a single string*/
        public String getArgStr()
        {
                String r="";
                for(int i=0;i<args.size();i++){
                        r+=(String)args.elementAt(i);
                        if(i<args.size())r+=" ";
                }
                return r;
        }
        
        /** return the blob of the message*/
        public byte[] getBlob()
        {
                if(blob!=null)return (byte[])blob.clone();
                else return null;
        }
}