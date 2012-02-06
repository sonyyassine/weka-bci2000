package wekabci.examples;

import java.awt.AWTException;
import java.awt.Image;
import java.awt.MenuItem;
import java.awt.PopupMenu;
import java.awt.SystemTray;
import java.awt.Toolkit;
import java.awt.TrayIcon;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import wekabci.server.WekaServer;

public class TrayIconExample {

	static WekaServer serverThread = null;
	static Image image = Toolkit.getDefaultToolkit().getImage("icons/Graph-16x16.png");
	static Image imageRunning = Toolkit.getDefaultToolkit().getImage("icons/Graph-16x16running.png");
	static TrayIcon trayIcon;
	static boolean isRunning = false;
	
	public static void main(String[] args) {
			

		if (SystemTray.isSupported()) {

			SystemTray tray = SystemTray.getSystemTray();
			image = Toolkit.getDefaultToolkit().getImage("icons/Graph-16x16.png");
			imageRunning = Toolkit.getDefaultToolkit().getImage("icons/Graph-16x16running.png");

			ActionListener exitListener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					System.out.println("Exiting...");
					System.exit(0);
				}
			};

			ActionListener startServerListener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					System.out.println("Starting server...");
					serverThread = new WekaServer();
					serverThread.start();
					trayIcon.setImage(imageRunning);
					isRunning = true;
				}
			};

			ActionListener stopServerListener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					System.out.println("Stoping the server...");
					if(serverThread != null)
						serverThread.setListening(false);
				}
			};

			PopupMenu popup = new PopupMenu();
			
			MenuItem ItemStartServer = new MenuItem("Start server");
			ItemStartServer.addActionListener(startServerListener);
			popup.add(ItemStartServer);
			
			MenuItem ItemStopServer = new MenuItem("Stop server");
			ItemStopServer.addActionListener(stopServerListener);
			popup.add(ItemStopServer);
			
			MenuItem defaultItem = new MenuItem("Exit");
			defaultItem.addActionListener(exitListener);
			popup.add(defaultItem);
			

			trayIcon = new TrayIcon(image, "WekaBCI Server", popup);

			ActionListener actionListener = new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					trayIcon.displayMessage("Action Event", 
							"An Action Event Has Been Performed!",
							TrayIcon.MessageType.INFO);
				}
			};

			trayIcon.setImageAutoSize(true);
			trayIcon.addActionListener(actionListener);

			try {
				tray.add(trayIcon);
			} catch (AWTException e) {
				System.err.println("TrayIcon could not be added.");
			}

		} else {

			//  System Tray is not supported

		}
	}
}
