#!/bin/sh

cat <<EOF >CI.java
import java.*;
import java.io.*;
public class CI {
	public static void main(String[]argv)
	{
		System.out.print("package org.bag;\npublic class Images{\n");
		int i;
		for(i=0;i<argv.length;i++){
			printfile(argv[i]);
		}
		System.out.print("}\n");
	}
	private static void printfile(String f)
	{
                FileInputStream fi;
                try{
        		fi=new FileInputStream(f);
        		byte b[]=new byte[fi.available()];
	        	fi.read(b);
        		fi.close();
                        System.out.print(" public static final byte[]"+getvar(f)+"={");
                        int i;
                        for(i=0;i<b.length;i++){
                                if(i>0)System.out.print(",");
                                System.out.print(b[i]);
                        }
                        System.out.print("};\n");
                }catch(FileNotFoundException e){
                        System.err.print("File "+f+" not found.\n");
                        System.exit(1);
                }catch(IOException e){
                        System.err.print("File "+f+": IO Error.\n");
                        System.exit(1);
                }
	}
        private static String getvar(String f)
        {
                String v=f;
                if(v.startsWith("./"))v=v.substring(2);
                return v.replace('.','_').replace('/','_');
        }
}
EOF

javac CI.java
java CI $*
rm CI.*
