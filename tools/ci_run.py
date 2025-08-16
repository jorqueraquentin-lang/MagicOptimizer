import os, sys, runpy

# Bridge env → argv for entry.py so it can be driven headlessly from CLI

phase = os.environ.get('MO_PHASE', 'Audit')
profile = os.environ.get('MO_PROFILE', 'Mobile_Low')
dry_run = os.environ.get('MO_DRYRUN', 'true').lower()
max_changes = os.environ.get('MO_MAXCHANGES', '100')
include = os.environ.get('MO_INCLUDE', '')
exclude = os.environ.get('MO_EXCLUDE', '')
use_selection = os.environ.get('MO_USESELECTION', 'false').lower()
categories = os.environ.get('MO_CATEGORIES', 'Textures,Meshes,Materials')

# Build argv exactly as entry.py expects
sys.argv = [
    'entry.py',
    phase,
    profile,
    dry_run,
    max_changes,
    include,
    exclude,
    use_selection,
    categories,
]

# entry.py is in the plugin's Python directory, not in tools/
entry_path = os.path.join(os.path.dirname(__file__), '..', 'HostProject', 'Content', 'Python', 'magic_optimizer', 'entry.py')
runpy.run_path(entry_path, run_name='__main__')


