import subprocess
import argparse
import sys
import os

def run_brogue_tests(seed_catalog_file, extra_args, max_level):

    output_seed_catalog_file = f'output_seed_catalog_{max_level}.txt'
    seed_catalog_cmd_args = f'--print-seed-catalog 1 25 {max_level}'

    # Generate the first 25 seeds
    if extra_args:
        brogue_command = f'./brogue {extra_args} {seed_catalog_cmd_args} > {output_seed_catalog_file}'
    else:
        brogue_command = f'./brogue {seed_catalog_cmd_args} > {output_seed_catalog_file}'
    print(f"Running {brogue_command}...")
    brogue_result = subprocess.run(brogue_command, shell=True, capture_output=True, text=True)

    # Run diff to compare the seed catalog files
    print (f"Comparing {seed_catalog_file} and {output_seed_catalog_file}...")

    diff_command = f"bash -c 'diff <(sed \"1,4d\" {seed_catalog_file}) <(sed \"1,4d\" {output_seed_catalog_file})'"
    diff_result = subprocess.run(diff_command, shell=True, capture_output=True, text=True)

    # Delete the output seed catalog file
    os.remove(output_seed_catalog_file)

    if brogue_result.returncode:
        print("Test run failure, seed catalog generation failed. Output:")
        print(brogue_result.stdout)
        sys.exit(1)

    if diff_result.returncode:
        print("Test run failure, seed catalog has changed.\nIf this was intentional, update the seed catalogs by running update_seed_catalogs.py.\nOutput:")
        print(diff_result.stdout)
        sys.exit(1)

    print("Seed catalog identical - test run successful")

def main():
    # Create the argument parser
    parser = argparse.ArgumentParser(description='Brogue Seed Compare Test Runner')
    parser.add_argument('seed_catalog', help='Seed catalog file to compare')
    parser.add_argument('max_level', help='Maximum level to generate seeds for')
    parser.add_argument('--extra_args', help='Extra command-line arguments to be passed to brogue (e.g. to select variant)')

    # Parse the command line arguments
    args = parser.parse_args()

    # Call the function to run the brogue tests
    run_brogue_tests(args.seed_catalog, args.extra_args, args.max_level)

if __name__ == '__main__':
    main()
