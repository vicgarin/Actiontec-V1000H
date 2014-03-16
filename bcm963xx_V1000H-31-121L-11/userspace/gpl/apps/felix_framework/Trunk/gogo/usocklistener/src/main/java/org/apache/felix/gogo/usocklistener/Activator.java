/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package org.apache.felix.gogo.usocklistener;

import java.util.Dictionary;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;
import org.osgi.service.command.CommandProcessor;
import org.osgi.service.command.CommandSession;
import org.osgi.service.command.Converter;
import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;
import org.osgi.util.tracker.ServiceTracker;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.Socket;
import gnu.java.net.local.LocalServerSocket;
import gnu.java.net.local.LocalSocketAddress;

public class Activator implements BundleActivator, Runnable
{
   private ServiceTracker commandProcessorTracker;
    private ServiceRegistration reg;
   private CommandSession session;
   Socket sock;
   InputStream is;
   OutputStream os;
   private Thread thread;

   public void start(final BundleContext context) throws Exception
   {
      commandProcessorTracker = processorTracker(context);
   }

   public void stop(BundleContext context) throws Exception
   {
    	if (thread != null)
        {
            thread.interrupt();
        }
      
        commandProcessorTracker.close();
    	reg.unregister();
   }
    
   public void run()
   {
      try
      {
         while(true)
         {
            if (is.available() > 0)
            {
                byte[] buf = new byte[256];            
                int read = is.read(buf);
                String rev = new String(buf, 0, read);        
                String[] cmd = rev.split("\\s");

                try
                {
                   session.execute(rev);       
                }
                catch (Exception e)
                {
                   os.write((cmd[0]+":ERR:"+e.getClass().getSimpleName() + ": " + e.getMessage()).getBytes());
                   //e.printStackTrace();
                }
            }   
            else
            {
               Thread.sleep(1000);
            }
         }   
      }
      catch (Exception e)
      {
         System.err.println(e.getClass().getSimpleName() + ": " + e.getMessage());
         e.printStackTrace();
      }
      finally
      {
         session.close();
      }
   }
    
    private void startUSockListener(BundleContext context, CommandProcessor processor)
    {
      // register converters
      reg = context.registerService(Converter.class.getName(), new Converters(context), null);
      
      try
      {
         LocalServerSocket server = new LocalServerSocket();
    	   	    
         server.bind(new LocalSocketAddress("/tmp/osgi/interfacesock"));
        
         while(true){ 
            sock = server.accept();
        
            is = sock.getInputStream();
            os = sock.getOutputStream();

            /*Saying hello to client*/
            os.write("Hello, dear Client".getBytes());	
            
            PrintStream ps = new PrintStream(os);
            
            session = processor.createSession(System.in, ps, System.err);
            thread = new Thread(this, "Unix Socket Listener");
            thread.start();
        }
      }
      catch (Exception e)
      {
         e.printStackTrace();
      }
    }

    private ServiceTracker processorTracker(BundleContext context)
    {
        ServiceTracker t = new ServiceTracker(context, CommandProcessor.class.getName(),
            null)
        {
            @Override
            public Object addingService(ServiceReference reference)
            {
                CommandProcessor processor = (CommandProcessor) super.addingService(reference);
                startUSockListener(context, processor);
                return processor;
            }

            @Override
            public void removedService(ServiceReference reference, Object service)
            {
                super.removedService(reference, service);
            }
        };

        t.open();
        return t;
    }
}
