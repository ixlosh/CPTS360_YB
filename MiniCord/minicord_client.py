import socket
import threading
import time
import tkinter as tk
from tkinter import scrolledtext, messagebox

# Configs
HOST = '127.0.0.1'
PORT = 9091

class MiniCordClient:
    def __init__(self, root):
        self.root = root
        self.root.title("MiniCord")
        self.root.geometry("800x500")
        
        # Networking setup
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((HOST, PORT))
        except Exception as e:
            messagebox.showerror("Connection Error", f"Could not connect to server: {e}")
            self.root.destroy()
            return

        # Login and main frames
        self.login_frame = tk.Frame(self.root, bg="purple")
        self.main_frame = tk.Frame(self.root, bg="#36393F")

        # Client state
        self.my_username = ""
        self.current_channel = ""
        self.online_users = set()
        self.default_channels = ["General", "Music", "Gaming", "Coding"]

        # Initialize the layouts
        self.setup_login_ui()
        self.setup_main_ui()

        # Show the login frame first
        self.login_frame.pack(fill=tk.BOTH, expand=True)

        # Concurrency/Multi-threading
        self.receive_thread = threading.Thread(target=self.receive_messages, daemon=True)
        self.receive_thread.start()

    def setup_login_ui(self):
        """Builds the login screen."""
        tk.Label(self.login_frame, text="Welcome to MiniCord", bg="purple", fg="white", font=("Arial", 24, "bold")).pack(pady=(120, 20))
        
        # Username text box
        self.username_entry = tk.Entry(self.login_frame, font=("Arial", 14), justify="center")
        self.username_entry.pack(pady=10, ipady=5)
        self.username_entry.bind("<Return>", lambda event: self.perform_login())
        
        # Login Button
        tk.Button(self.login_frame, text="Login", bg="#7289DA", fg="white", font=("Arial", 14, "bold"), width=15, command=self.perform_login).pack(pady=20)

    def setup_main_ui(self):
        """Builds the chat interface."""
        # Left Sidebar
        self.left_frame = tk.Frame(self.main_frame, width=150, bg="#2C2F33")
        self.left_frame.pack(side=tk.LEFT, fill=tk.Y)
        tk.Label(self.left_frame, text="Channels", bg="#2C2F33", fg="white", font=("Arial", 12, "bold")).pack(pady=10)
        
        # Dynamic channel list
        self.channels_frame = tk.Frame(self.left_frame, bg="#2C2F33")
        self.channels_frame.pack(fill=tk.X)

        self.channels = [] 
        self.channel_labels = {} 

        # Join channel button at the bottom of the left sidebar
        self.add_channel_btn = tk.Button(self.left_frame, text="+ Join Channel", bg="#7289DA", fg="white", command=self.prompt_join_channel)
        self.add_channel_btn.pack(side=tk.BOTTOM, pady=20, padx=10, fill=tk.X)
        
        # Right Sidebar
        self.right_frame = tk.Frame(self.main_frame, width=150, bg="#2C2F33")
        self.right_frame.pack(side=tk.RIGHT, fill=tk.Y)
        tk.Label(self.right_frame, text="Users", bg="#2C2F33", fg="white", font=("Arial", 12, "bold")).pack(pady=10)
        self.users_listbox = tk.Listbox(self.right_frame, bg="#2C2F33", fg="white", bd=0, highlightthickness=0)
        self.users_listbox.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        self.users_listbox.bind("<<ListboxSelect>>", self.handle_user_click)

        # Center Area
        self.center_frame = tk.Frame(self.main_frame, bg="#36393F")
        self.center_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        self.chat_history = scrolledtext.ScrolledText(self.center_frame, state='disabled', bg="#36393F", fg="white", font=("Arial", 10))
        self.chat_history.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        self.entry_frame = tk.Frame(self.center_frame, bg="#36393F")
        self.entry_frame.pack(fill=tk.X, padx=10, pady=(0, 10))
        
        # Main chat text box
        self.msg_entry = tk.Entry(self.entry_frame, bg="#40444B", fg="white", font=("Arial", 12), insertbackground="white")
        self.msg_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, ipady=5)
        self.msg_entry.bind("<Return>", self.send_message)
        
        self.send_btn = tk.Button(self.entry_frame, text="Send", bg="#7289DA", fg="white", command=self.send_message)
        self.send_btn.pack(side=tk.RIGHT, padx=(10, 0))

    def perform_login(self):
        """Logs the user into MiniCord."""
        username = self.username_entry.get().strip()
        if username:
            self.my_username = username
            self.online_users = {username}
            self.current_channel = "General" # Active channel, default is #General

            for channel in self.default_channels:
                self.add_channel_to_ui(channel)

            for lbl in self.channel_labels.values():
                lbl.config(fg="#8E9297", bg="#2C2F33")
            
            if "General" in self.channel_labels:
                self.channel_labels["General"].config(fg="#FFFFFF", bg="#40444B")

            self.update_users_listbox() # Immediate update to ensure user list is never empty

            login_command = f"/login {username}\n"
            roster_command = "/users General\n" 
            
            self.sock.sendall(login_command.encode('utf-8'))
            time.sleep(0.1) # Small delay to ensure the server processes the login before the roster request
            self.sock.sendall(roster_command.encode('utf-8'))
            
            self.login_frame.pack_forget()
            self.main_frame.pack(fill=tk.BOTH, expand=True)
            self.msg_entry.focus_set() 

    def display_message(self, message):
        """Inserts a message into the chat history text box."""
        self.chat_history.config(state='normal')
        self.chat_history.insert(tk.END, message)
        self.chat_history.see(tk.END) # Auto-scroll to the bottom, consistent with typical chat behavior
        self.chat_history.config(state='disabled')

    def send_message(self, event=None):
        """Grabs text from the entry box and sends it to the server."""
        msg = self.msg_entry.get()
        if msg:
            try:
                # Proper formatting for direct messages
                if getattr(self, 'current_channel', '').startswith("@"):
                    target_user = self.current_channel[1:]
                    if not msg.startswith("/"):
                        formatted_msg = f"/msg {target_user} {msg}\n"
                        self.sock.sendall(formatted_msg.encode('utf-8'))
                        self.display_message(f"[You to {target_user}]: {msg}\n")
                    else:
                        self.sock.sendall((msg + "\n").encode('utf-8'))
                else:
                    self.sock.sendall((msg + "\n").encode('utf-8'))
                    if not msg.startswith("/"):
                        self.display_message(f"[You]: {msg}\n")
                        
                self.msg_entry.delete(0, tk.END)
            except Exception as e:
                self.display_message(f"[System]: Failed to send message. {e}\n")

    def receive_messages(self):
        """Background network loop essential to providing the desired parsing."""
        buffer = ""
        
        while True:
            try:
                data = self.sock.recv(1024)
                if not data:
                    self.display_message("[System]: Disconnected from server.\n")
                    self.sock.close()
                    break
                
                buffer += data.decode('utf-8')
                
                # Intercept the server messages and act appropriately (dynamic UI updates)
                while "\n" in buffer:
                    message, buffer = buffer.split("\n", 1)
                    
                    # User list feedback
                    if message.startswith("[Server]: Users in #"):
                        parts = message.split(":", 2)
                        if len(parts) >= 3:
                            channel_part = parts[1].split("#")[-1].strip()
                            if channel_part == getattr(self, 'current_channel', ''):
                                names_string = parts[2].strip()
                                self.online_users = set(names_string.split())
                                self.update_users_listbox()
                        continue 
                    
                    # Presence alerts
                    if message.startswith("[Server]:"):
                        body = message.split("[Server]:", 1)[-1].strip()
                        
                        if "has come online in #" in body:
                            parts = body.split(" has come online in #")
                            user_part = parts[0].strip()
                            channel_part = parts[1].strip(" .\n\r")
                            
                            if channel_part == getattr(self, 'current_channel', ''):
                                self.online_users.add(user_part)
                                self.update_users_listbox()
                                
                        elif "has gone offline from #" in body:
                            parts = body.split(" has gone offline from #")
                            user_part = parts[0].strip()
                            channel_part = parts[1].strip(" .\n\r")
                            
                            if channel_part == getattr(self, 'current_channel', ''):
                                if user_part in self.online_users:
                                    self.online_users.remove(user_part)
                                    self.update_users_listbox()

                        # Channel switch alerts
                        elif "has switched from #" in body and " to #" in body:
                            parts = body.split(" has switched from #")
                            user_part = parts[0].strip()

                            channels_part = parts[1] 

                            channel_splits = channels_part.split(" to #")
                            old_channel = channel_splits[0].strip()
                            new_channel = channel_splits[1].strip(" .\n\r")
                            
                            # If they left the room we are in, remove them
                            if old_channel == getattr(self, 'current_channel', ''):
                                if user_part in self.online_users:
                                    self.online_users.remove(user_part)
                                    self.update_users_listbox()      
                            # If they joined the room we are in, add them
                            elif new_channel == getattr(self, 'current_channel', ''):
                                self.online_users.add(user_part)
                                self.update_users_listbox()
                            else:
                                continue # If the switch doesn't affect our current channel, ignore it

                    # Direct messaging
                    if message.startswith("[DM from "):
                        parts = message.split("]: ", 1)
                        if len(parts) == 2:
                            sender = parts[0].replace("[DM from ", "")
                            # If we are not currently focused on their DM tab, notify us!
                            if getattr(self, 'current_channel', '') != f"@{sender}":
                                self.create_dm_tab(sender, is_notification=True)

                    # Print regular chat messages to the screen
                    self.display_message(message + "\n")
                    
            except Exception as e:
                print(f"Network error: {e}")
                self.sock.close()
                break

    # Easy Direct Messaging access
    def handle_user_click(self, event):
        selection = self.users_listbox.curselection()
        if not selection:
            return
        user = self.users_listbox.get(selection)
        user = user.replace(" (You)", "")
        if user == self.my_username:
            return # Can't DM yourself
            
        self.create_dm_tab(user, is_notification=False)
        self.switch_channel(f"@{user}")
        self.users_listbox.selection_clear(0, tk.END)

    # DM tab creation
    def create_dm_tab(self, user, is_notification=False):
        dm_channel = f"@{user}"
        
        if dm_channel in self.channel_labels:
            if is_notification and self.current_channel != dm_channel:
                self.channel_labels[dm_channel].config(fg="white", bg="orange")
            return
        lbl = tk.Label(self.channels_frame, text=dm_channel,
                       bg="orange" if is_notification else "#2C2F33",
                       fg="white" if is_notification else "#8E9297",
                       font=("Arial", 11), cursor="hand2")
        lbl.pack(anchor="w", padx=20, pady=2)
        lbl.bind("<Button-1>", lambda e, c=dm_channel: self.switch_channel(c))
        self.channel_labels[dm_channel] = lbl

    def prompt_join_channel(self):
        """Opens a pop-up window to enter a new channel name."""
        join_win = tk.Toplevel(self.root)
        join_win.title("Join Channel")
        join_win.geometry("300x150")
        join_win.configure(bg="purple") 
        join_win.grab_set()

        tk.Label(join_win, text="Enter Channel Name:", bg="purple", fg="white", font=("Arial", 12, "bold")).pack(pady=15)

        channel_entry = tk.Entry(join_win, font=("Arial", 12), justify="center")
        channel_entry.pack(pady=5, ipady=3)
        channel_entry.focus_set()

        def submit_channel(event=None):
            raw_input = channel_entry.get().strip()
            
            if raw_input:
                new_channel = raw_input.split()[0]
                
                # If we haven't hit the limit, switch to the new channel (the limit is really just proprietary)
                if new_channel not in self.channels and len(self.channels) >= 10:
                    messagebox.showwarning("Channel Limit", "You can only join up to 10 channels.")
                else:
                    self.switch_channel(new_channel)
            
            join_win.destroy()

        channel_entry.bind("<Return>", submit_channel)
        tk.Button(join_win, text="Join", bg="#7289DA", fg="white", font=("Arial", 10, "bold"), command=submit_channel).pack(pady=10)

    def add_channel_to_ui(self, channel_name):
        """Adds a channel to the sidebar or switches if it already exists."""
        if channel_name not in self.channels:
            # Check our 10 channel limit
            if len(self.channels) >= 10:
                messagebox.showwarning("Channel Limit", "You can only join up to 10 channels.")
                return False 
            
            self.channels.append(channel_name)
            
            lbl = tk.Label(self.channels_frame, text=f"# {channel_name}", bg="#2C2F33", fg="#8E9297", font=("Arial", 11), cursor="hand2")
            lbl.pack(anchor="w", padx=20, pady=2)
            
            lbl.bind("<Button-1>", lambda e, c=channel_name: self.switch_channel(c))
            
            self.channel_labels[channel_name] = lbl

        # Loop through all channels and highlight active one
        for name, lbl in self.channel_labels.items():
            if name == channel_name:
                lbl.config(fg="#FFFFFF", bg="#40444B")
            else:
                lbl.config(fg="#8E9297", bg="#2C2F33")

        return True

    def switch_channel(self, channel_name):
        """Sends the join command to the server and updates UI."""
        # If the user clicks the channel they are already in, do nothing.
        if channel_name == getattr(self, 'current_channel', ''):
            return

        old_channel = getattr(self, 'current_channel', '')

        if old_channel.startswith("@") and old_channel != channel_name:
            # If we are switching away from a DM tab, remove it from the sidebar
            # DM tabs are temporary and do not persist like proper channels
            if old_channel in self.channel_labels:
                self.channel_labels[old_channel].destroy()
                del self.channel_labels[old_channel]
                if old_channel in self.channels:
                    self.channels.remove(old_channel)
        
        self.current_channel = channel_name

        if channel_name.startswith("@"):
            self.chat_history.config(state='normal')
            self.chat_history.delete(1.0, tk.END)
            self.chat_history.insert(tk.END, f"--- Direct Message with {channel_name[1:]} ---\n")
            self.chat_history.config(state='disabled')
        else:
            join_command = f"/join {channel_name}\n"
            roster_command = f"/users {channel_name}\n"
            try:
                self.sock.sendall(join_command.encode('utf-8'))
                time.sleep(0.1) # Small delay to ensure no race conditions
                self.sock.sendall(roster_command.encode('utf-8'))

                self.add_channel_to_ui(channel_name)
            
                self.chat_history.config(state='normal')
                self.chat_history.delete(1.0, tk.END)
                self.chat_history.config(state='disabled')
            except Exception as e:
                self.display_message(f"[System]: Failed to switch channel. {e}\n")

        # Highlight the active channel in the sidebar
        for name, lbl in self.channel_labels.items():
            if name == channel_name:
                lbl.config(fg="#FFFFFF", bg="#40444B")
            else:
                lbl.config(fg="#8E9297", bg="#2C2F33")

    def update_users_listbox(self):
        """Updates the users list."""
        self.users_listbox.delete(0, tk.END) # Start by cleaning
        
        # Redraw the list alphabetically
        for user in sorted(self.online_users):
            if user == getattr(self, 'my_username', ''):
                self.users_listbox.insert(tk.END, f"{user} (You)")
                self.users_listbox.itemconfig(tk.END, {'fg': '#7289DA'}) # Special color for client
            else:
                self.users_listbox.insert(tk.END, user)

if __name__ == "__main__":
    root = tk.Tk()
    app = MiniCordClient(root)
    root.protocol("WM_DELETE_WINDOW", lambda: (app.sock.close(), root.destroy()))
    root.mainloop()