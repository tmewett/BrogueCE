import subprocess
import argparse
import sys

def create_brogue_seed_catalog_file(output_seed_catalog_file, extra_args, max_level):

    seed_catalog_cmd_args = f'--print-seed-catalog 1 25 {max_level}'

    # Generate the first 25 seeds
    if extra_args:
        brogue_command = f'./bin/brogue {extra_args} {seed_catalog_cmd_args} > {output_seed_catalog_file}'
    else:
        brogue_command = f'./bin/brogue {seed_catalog_cmd_args} > {output_seed_catalog_file}'
    print(f"Running {brogue_command}...")
    brogue_result = subprocess.run(brogue_command, shell=True, capture_output=True, text=True)

    if brogue_result.returncode:
        print("Seed catalog generation failed. Output:")
        print(brogue_result.stdout)
        sys.exit(1)

    print("Seed catalog creation complete.")

def main():
    parser = argparse.ArgumentParser(description='Brogue Seed Catalog Updater for Brogue Seed Compare Test Runner. Run this script to update the seed catalog files used by the Brogue Seed Compare Test Runner. This should be done after a change to the Brogue source code that changes the dungeon generation. Assumes brogue compiled binary available in ./bin/brogue')
    parser.parse_args()

    # Create the seed catalog files used in the compare test runner
    create_brogue_seed_catalog_file('test/seed_catalogs/seed_catalog_brogue.txt', None, 40)
    create_brogue_seed_catalog_file('test/seed_catalogs/seed_catalog_rapid_brogue.txt', "--variant rapid_brogue", 10)

if __name__ == '__main__':
    main()
