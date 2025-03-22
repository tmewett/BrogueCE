# PowerShell script to view Brogue DM AI logs in a browser
# This script creates a simple HTML viewer for the Markdown log files

# Configuration
$LOGS_DIR = Join-Path $PSScriptRoot "logs"
$HTML_DIR = Join-Path $PSScriptRoot "html"
$INDEX_PATH = Join-Path $HTML_DIR "index.html"
$PORT = 8080

# ANSI colors for PowerShell output
$ESC = [char]27
$COLORS = @{
    RESET = "$ESC[0m"
    BRIGHT = "$ESC[1m"
    RED = "$ESC[31m"
    GREEN = "$ESC[32m"
    YELLOW = "$ESC[33m"
    BLUE = "$ESC[34m"
    MAGENTA = "$ESC[35m"
    CYAN = "$ESC[36m"
}

# Create HTML directory if it doesn't exist
if (-not (Test-Path $HTML_DIR)) {
    Write-Host "$($COLORS.CYAN)Creating HTML directory at $HTML_DIR$($COLORS.RESET)"
    New-Item -Path $HTML_DIR -ItemType Directory -Force | Out-Null
}

# Function to create HTML file for each log
function Convert-MarkdownToHtml {
    param (
        [string]$mdPath,
        [string]$htmlPath
    )
    
    $mdContent = Get-Content -Path $mdPath -Raw
    $htmlContent = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Brogue DM AI Log Viewer</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/highlight.js@11.7.0/styles/github-dark.css">
    <style>
        body {
            background-color: #1e1e1e;
            color: #e0e0e0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: #252526;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        h1, h2, h3 {
            color: #f8f8f8;
        }
        h2 {
            border-bottom: 1px solid #444;
            padding-bottom: 5px;
            margin-top: 25px;
        }
        h3 {
            margin-top: 20px;
            color: #dcdcaa;
        }
        blockquote {
            border-left: 4px solid #569cd6;
            padding: 10px 15px;
            margin: 15px 0;
            background-color: #2d2d30;
            border-radius: 4px;
            font-style: italic;
        }
        code {
            background-color: #1e1e1e;
            color: #ce9178;
            padding: 2px 5px;
            border-radius: 3px;
        }
        pre {
            background-color: #1e1e1e;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
        }
        .navbar {
            background-color: #333;
            padding: 10px 0;
            margin-bottom: 20px;
        }
        .navbar a {
            color: white;
            text-decoration: none;
            margin: 0 15px;
        }
        .event-icon {
            font-size: 1.2em;
            margin-right: 8px;
        }
        .timestamp {
            color: #777;
            font-size: 0.9em;
        }
        .back-link {
            display: inline-block;
            margin-bottom: 20px;
            color: #569cd6;
            text-decoration: none;
        }
        .back-link:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="navbar">
            <a href="index.html">Log Index</a>
            <span style="float: right;">Brogue DM AI</span>
        </div>
        
        <a href="index.html" class="back-link">← Back to Index</a>
        
        <div id="content">
            <div id="markdown-content"></div>
        </div>
    </div>
    
    <script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/highlight.js@11.7.0/lib/core.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/highlight.js@11.7.0/lib/languages/json.min.js"></script>
    
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            // Parse and render markdown
            const mdContent = `$($mdContent -replace '`', '\`' -replace '"', '\"' -replace "`r`n", '\n')`;
            document.getElementById('markdown-content').innerHTML = marked.parse(mdContent);
            
            // Add icons to headers based on content
            const headers = document.querySelectorAll('h3');
            headers.forEach(header => {
                const text = header.textContent;
                let icon = '';
                
                if (text.includes('Encountered')) {
                    icon = '👹';
                } else if (text.includes('Defeated') || text.includes('killed')) {
                    icon = '💀';
                } else if (text.includes('Found')) {
                    icon = '💎';
                } else if (text.includes('Player died')) {
                    icon = '☠️';
                } else if (text.includes('Descended') || text.includes('depth')) {
                    icon = '🗺️';
                }
                
                if (icon) {
                    header.innerHTML = `<span class="event-icon">${icon}</span> ${header.innerHTML}`;
                }
            });
            
            // Highlight code blocks
            document.querySelectorAll('pre code').forEach((block) => {
                hljs.highlightBlock(block);
            });
        });
    </script>
</body>
</html>
"@
    
    Set-Content -Path $htmlPath -Value $htmlContent
}

# Create index HTML file
function Create-IndexHtml {
    $logFiles = Get-ChildItem -Path $LOGS_DIR -Filter "events_*.md" | Sort-Object LastWriteTime -Descending
    
    $logsHtml = ""
    foreach ($logFile in $logFiles) {
        $sessionDate = $logFile.LastWriteTime.ToString("yyyy-MM-dd HH:mm:ss")
        $sessionName = $logFile.BaseName -replace "events_", ""
        $htmlFileName = "$($logFile.BaseName).html"
        $htmlPath = Join-Path $HTML_DIR $htmlFileName
        
        # Convert markdown to HTML
        Convert-MarkdownToHtml -mdPath $logFile.FullName -htmlPath $htmlPath
        
        # Add entry to index
        $logsHtml += @"
        <tr>
            <td><a href="$htmlFileName">Session $sessionName</a></td>
            <td>$sessionDate</td>
            <td>
                <a href="$htmlFileName" class="btn btn-sm btn-primary">View</a>
            </td>
        </tr>
"@
    }
    
    $indexHtml = @"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Brogue DM AI Log Index</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css">
    <style>
        body {
            background-color: #1e1e1e;
            color: #e0e0e0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: #252526;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        }
        h1 {
            color: #569cd6;
            text-align: center;
            margin-bottom: 30px;
        }
        .table {
            color: #e0e0e0;
        }
        .table a {
            color: #569cd6;
            text-decoration: none;
        }
        .table a:hover {
            text-decoration: underline;
        }
        .navbar {
            background-color: #333;
            padding: 10px 0;
            margin-bottom: 20px;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="navbar">
            <span>Brogue DM AI Log Viewer</span>
        </div>
        
        <h1>Dungeon Master AI Logs</h1>
        
        <div class="table-responsive">
            <table class="table table-dark table-striped">
                <thead>
                    <tr>
                        <th>Session</th>
                        <th>Date</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    $logsHtml
                </tbody>
            </table>
        </div>
    </div>
</body>
</html>
"@
    
    Set-Content -Path $INDEX_PATH -Value $indexHtml
}

# Main execution
Write-Host "$($COLORS.BRIGHT)$($COLORS.MAGENTA)Brogue DM AI Log Viewer$($COLORS.RESET)"

# Check if logs exist
if (-not (Test-Path $LOGS_DIR) -or (Get-ChildItem -Path $LOGS_DIR -Filter "events_*.md").Count -eq 0) {
    Write-Host "$($COLORS.RED)No log files found in $LOGS_DIR$($COLORS.RESET)"
    exit 1
}

# Create HTML files
Write-Host "$($COLORS.CYAN)Converting log files to HTML...$($COLORS.RESET)"
Create-IndexHtml

# Start a simple HTTP server if possible
Write-Host "$($COLORS.GREEN)HTML files created in $HTML_DIR$($COLORS.RESET)"

# Try to open the index file in the default browser
Write-Host "$($COLORS.YELLOW)Opening log viewer in browser...$($COLORS.RESET)"
Start-Process $INDEX_PATH 