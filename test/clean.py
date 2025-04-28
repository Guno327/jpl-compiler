import re
import argparse


def process_assembly_file(input_path, output_path):
    try:
        # Read the input file
        with open(input_path, "r") as infile:
            lines = infile.readlines()

        # Process each line
        processed_lines = []
        for line in lines:
            # Remove tabs
            line = line.replace("\t", "")
            # Remove comments (anything starting with ';')
            line = re.sub(r";.*$", "", line)
            # Strip trailing whitespace but preserve the newline
            line = line.rstrip() + "\n"
            # Only add non-empty lines (after stripping comments and tabs)
            if line.strip():
                processed_lines.append(line)

        # Write to the output file
        with open(output_path, "w") as outfile:
            outfile.writelines(processed_lines)

        print(f"Successfully processed {input_path} and saved to {output_path}")

    except FileNotFoundError:
        print(f"Error: Input file {input_path} not found.")
    except Exception as e:
        print(f"Error: An unexpected error occurred: {e}")


def main():
    # Set up argument parser
    parser = argparse.ArgumentParser(
        description="Remove tabs and comments from an assembly file."
    )
    parser.add_argument("input_file", help="Path to the input assembly file")
    parser.add_argument("output_file", help="Path to the output processed file")

    # Parse arguments
    args = parser.parse_args()

    # Process the file
    process_assembly_file(args.input_file, args.output_file)


if __name__ == "__main__":
    main()
