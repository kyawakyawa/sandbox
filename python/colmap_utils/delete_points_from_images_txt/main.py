from dataclasses import dataclass
import tyro


@dataclass
class Config:
    input: str
    output: str


def main(config: Config):
    # Read input file
    with open(config.input, 'r') as file:
        lines = file.readlines()
    
    # Process the file according to requirements
    processed_lines = []
    non_comment_line_count = 0
    
    for line in lines:
        # Skip comment lines (lines starting with #)
        if line.strip().startswith('#'):
            processed_lines.append(line)  # Keep comments in output
            continue
        
        # Count this as a non-comment line
        non_comment_line_count += 1
        
        # For odd-numbered lines, keep them as they are
        if non_comment_line_count % 2 == 1:
            processed_lines.append(line)
        # For even-numbered lines, replace with an empty line
        else:
            processed_lines.append('\n')
    
    # Write to output file
    with open(config.output, 'w') as file:
        file.writelines(processed_lines)


if __name__ == "__main__":
    config = tyro.cli(Config)
    main(config)
