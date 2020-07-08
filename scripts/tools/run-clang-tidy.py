import os
import sys
import subprocess
import argparse
import multiprocessing
import functools
import fnmatch

def find_sources(inputs, extensions=None, exclude=None):
    if extensions is None:
        extensions = []
    if exclude is None:
        exclude = []

    result = []
    for path in inputs:
        if os.path.isfile(path):
            result.append(os.path.abspath(path).replace("\\","/"))
        elif os.path.isdir(path):
            for dirpath, dnames, fnames in os.walk(path):
                fpaths = [os.path.join(dirpath, fname) for fname in fnames]
                for fpath in fpaths:
                    ext = os.path.splitext(fpath)[1][1:]
                    if ext not in extensions:
                        continue
                    if any(map(functools.partial(fnmatch, fpath), exclude)):
                        continue
                    result.append(os.path.abspath(fpath).replace("\\","/"))
            
    return result

def run_clang_tidy(clang_tidy_path, build_path, quiet, apply, sources):
    args = [clang_tidy_path, '-p', build_path]
    if quiet:
        args.append('--quiet')
    if apply:
        args.append('--fix')

    chunk_size = 16
    chunks = [sources[i:i + chunk_size] for i in range(0, len(sources), chunk_size)]

    success = True
    for chunk in chunks:
        print(args+chunk, flush=True)
        success = subprocess.run(args + chunk) and success
    return success

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('paths', metavar='PATH', nargs='+')
    parser.add_argument('--build', '-b',
        metavar='BUILD',
        help='path to the build directory',
        default='.')
    parser.add_argument(
        '--clang-tidy',
        metavar='EXECUTABLE',
        help='path to the clang-tidy executable',
        default='clang-tidy')
    parser.add_argument(
        '--quiet', '-q',
        action='store_true',
        help='quiet default output')
    parser.add_argument(
        '-j',
        metavar='JOBS',
        help='number of jobs',
        default=0)
    parser.add_argument(
        '--apply', '-A',
        action='store_true',
        help='apply fixes')
    parser.add_argument(
        '-e',
        '--exclude',
        metavar='PATTERN',
        action='append',
        default=[],
        help='exclude glob patterns from input paths')

    args = parser.parse_args()

    inputs = args.paths
    sources = find_sources(inputs, extensions=['c', 'cpp'], exclude=args.exclude)
    
    jobs = args.j
    if jobs == 0:
        jobs = multiprocessing.cpu_count() + 1
    jobs = min(len(sources), jobs)

    wrap_tidy = functools.partial(run_clang_tidy, args.clang_tidy, args.build, args.quiet, args.apply)

    if jobs == 1:
        chunk_size = 16
        chunks = [sources[i:i + chunk_size] for i in range(0, len(sources), chunk_size)]
        results = functools.map(wrap_tidy, chunks)
    else:
        pool = multiprocessing.Pool(processes=jobs)

        files_per_job = len(sources) // jobs
        job_sources = [sources[i:i + files_per_job] for i in range(0, len(sources), files_per_job)]

        results = []
        for result in pool.map(wrap_tidy, job_sources):
            results.append(result)

        pool.close()
        pool.join()

    return 0 if len(results) == 0 or all(results) else 1

if __name__ == '__main__':
    sys.exit(main())
