use std::path::Path;
use std::thread;

mod watcher;
mod commands;

use watcher::FileWatcher;
use commands::CommandHandler;

fn main() -> std::io::Result<()> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("Usage: {} <project_directory>", args[0]);
        std::process::exit(1);
    }

    let project_path = Path::new(&args[1]);
    if !project_path.exists() || !project_path.is_dir() {
        eprintln!("Error: Specified path does not exist or is not a directory");
        std::process::exit(1);
    }

    let ignore_dirs = vec![
        "node_modules",
        ".git",
        "target",
        "dist",
        "build",
        "bin",
        "obj",
        ".o",
        "dev-server",
    ];

    println!("Watching directory: {}", project_path.display());
    println!("Ignoring directories: {:?}", ignore_dirs);
    println!("Available commands:");
    println!("  rebuild - Force a rebuild");
    println!("  status - Show current status");
    println!("  quit   - Exit the server");
    
    let project_path_clone = project_path.to_path_buf();
    let mut watcher = FileWatcher::new(project_path, ignore_dirs)?;
    
    // Spawn file watching thread
    let watcher_handle = thread::spawn(move || {
        watcher.watch_directory()
    });

    // Main thread handles commands
    let mut command_handler = CommandHandler::new(project_path_clone);
    command_handler.run_command_loop()?;

    // Clean up
    watcher_handle.join().expect("Watcher thread panicked")?;
    
    Ok(())
}