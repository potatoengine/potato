import sys
import glob
import argparse
import os
import shutil
import datetime

def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--name', required=True, type=str, help='Name of new project')
    parser.add_argument('--template', type=str, help='Source template directory')
    parser.add_argument('--dest', type=str, help='Destination directory')
    args = parser.parse_args(argv[1:])

    name = args.name

    year = datetime.datetime.now().year

    if args.template is not None:
        source_dir = args.template
    else:
        source_dir = os.path.join(os.path.dirname(argv[0]), 'files')

    if args.dest is not None:
        dest_dir = args.dest
    else:
        dest_dir = os.path.relpath(os.path.join(source_dir, '..', '..', 'sources', 'library'), source_dir)
    dest_dir = os.path.join(dest_dir, name)

    print('Creating `{0}` at `{1}` from `{2}`'.format(name, dest_dir, source_dir))

    for source_file in glob.iglob(pathname=os.path.join(source_dir, '**', '*'), recursive=True):
        if os.path.isdir(source_file):
            continue

        relative_path=os.path.relpath(source_file, source_dir)
        dir_name=os.path.dirname(relative_path)

        target_dir_name=os.path.join(dest_dir, dir_name).replace('__name__', name)

        root_file, ext = os.path.splitext(relative_path)

        if not os.path.isdir(target_dir_name):
            os.makedirs(target_dir_name)

        if ext == '.in':
            target_file=os.path.join(dest_dir, root_file).replace('__name__', name)
            with open(source_file, 'rt') as template_file:
                template = template_file.read()

            processed = template.replace('@NAME@', name).replace('@NAME_UPPER@', name.upper()).replace('@YEAR@', str(year))

            print('write template', target_file)
            with open(target_file, 'wt') as output_file:
               output_file.write(processed)
        else:
            target_file=os.path.join(dest_dir, relative_path).replace('__name__', name)
            print('copy to', target_file)
            shutil.copyfile(src=source_file, dst=target_file)

if __name__ == '__main__':
    main(sys.argv)
