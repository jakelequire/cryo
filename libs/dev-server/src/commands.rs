use std::io::{self, Write};
use std::path::PathBuf;
use std::process::Command;

pub struct CommandHandler {
    project_path: PathBuf,
}

impl CommandHandler {
    pub fn new(project_path: PathBuf) -> Self {
        CommandHandler { project_path }
    }

    pub fn run_command_loop(&mut self) -> io::Result<()> {
        let stdin = io::stdin();
        let mut input = String::new();
        
        loop {
            print!("> ");
            io::stdout().flush()?;
            
            input.clear();
            stdin.read_line(&mut input)?;
            
            let command = input.trim();
            
            match command {
                "rebuild" => self.force_rebuild()?,
                "status" => self.show_status()?,
                "quit" | "exit" => {
                    println!("Shutting down server...");
                    break;
                }
                "" => continue,
                _ => println!("Unknown command: {}", command),
            }
        }
        
        Ok(())
    }

    fn force_rebuild(&self) -> io::Result<()> {
        println!("Forcing rebuild...");
        let output = Command::new("make")
            .current_dir(&self.project_path)
            .output()?;
            
        if output.status.success() {
            println!("Build successful!");
            println!("{}", String::from_utf8_lossy(&output.stdout));
        } else {
            eprintln!("Build failed!");
            eprintln!("{}", String::from_utf8_lossy(&output.stderr));
        }
        Ok(())
    }

    fn show_status(&self) -> io::Result<()> {
        println!("Current project directory: {}", self.project_path.display());
        // Add more status information as needed
        Ok(())
    }
}