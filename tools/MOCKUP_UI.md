import React, { useEffect, useMemo, useRef, useState } from "react";
import { motion, AnimatePresence } from "framer-motion";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { Progress } from "@/components/ui/progress";
import { Tooltip, TooltipContent, TooltipProvider, TooltipTrigger } from "@/components/ui/tooltip";
import { DropdownMenu, DropdownMenuCheckboxItem, DropdownMenuContent, DropdownMenuItem, DropdownMenuTrigger } from "@/components/ui/dropdown-menu";
import { Dialog, DialogContent, DialogDescription, DialogFooter, DialogHeader, DialogTitle } from "@/components/ui/dialog";
import { Sheet, SheetContent, SheetDescription, SheetFooter, SheetHeader, SheetTitle } from "@/components/ui/sheet";
import { Input } from "@/components/ui/input";
import { Checkbox } from "@/components/ui/checkbox";
import { Switch } from "@/components/ui/switch";
import { Separator } from "@/components/ui/separator";
import { ScrollArea } from "@/components/ui/scroll-area";
import { Label } from "@/components/ui/label";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import {
  Play,
  Wand2,
  Eye,
  ChevronRight,
  ChevronDown,
  Gamepad2,
  Cpu,
  Smartphone,
  FileDown,
  ClipboardList,
  ClipboardCheck,
  Search as SearchIcon,
  Filter,
  Link2,
  X,
  Gauge,
  Activity,
  HardDrive
} from "lucide-react";
import {
  LineChart,
  Line,
  CartesianGrid,
  XAxis,
  YAxis,
  Tooltip as RTooltip,
  ResponsiveContainer,
  BarChart,
  Bar,
  Legend,
  RadialBarChart,
  RadialBar,
  PolarAngleAxis,
} from "recharts";

// ---------- Helpers and mock data (pure JS) ----------
const TARGET_CHIPS = [
  { id: "ps5", label: "PS5", icon: <Gamepad2 className="h-3.5 w-3.5" /> },
  { id: "xbox", label: "Xbox", icon: <Gamepad2 className="h-3.5 w-3.5" /> },
  { id: "pc", label: "PC", icon: <Cpu className="h-3.5 w-3.5" /> },
  { id: "quest", label: "Quest", icon: <Smartphone className="h-3.5 w-3.5" /> },
  { id: "switch", label: "Switch", icon: <Smartphone className="h-3.5 w-3.5" /> }
];

const PRESETS = ["PC High", "Console Balanced", "Mobile Low", "VR Crisp UI", "Cinematic Preview"];

function rndInt(min, max) { return Math.floor(Math.random() * (max - min + 1)) + min; }

function makeTextureRow(i) {
  const sizes = ["512", "1k", "2k", "4k"];
  const groups = ["World", "UI", "Effects", "Characters", "Props"];
  const comp = ["Default", "Masks", "UserInterface", "Normal", "HDR"];
  const s = sizes[rndInt(0, sizes.length - 1)];
  return {
    id: `tex_${i}`,
    path: `/Game/Props/T_${i.toString().padStart(3, "0")}`,
    size: s,
    group: groups[rndInt(0, groups.length - 1)],
    compression: comp[rndInt(0, comp.length - 1)],
    sRGB: Math.random() > 0.5,
    mips: Math.random() > 0.3,
    vt: Math.random() > 0.6,
    lod: rndInt(0, 2),
    referencers: rndInt(0, 15),
    issues: rndInt(0, 3),
  };
}

const MOCK_TEXTURES = Array.from({ length: 80 }, (_, i) => makeTextureRow(i + 1));

const healthHistory = Array.from({ length: 20 }, (_, i) => ({ t: i, score: rndInt(60, 90) }));

const beforeAfter = [
  { name: "Disk MB", Before: 3120, After: 2710 },
  { name: "GPU Mem MB", Before: 2240, After: 1820 },
  { name: "Draw Calls", Before: 14500, After: 11700 },
  { name: "Shader Cost", Before: 1.0, After: 0.78 }
];

const mockRuns = [
  { id: "run_01", date: "2025-09-10 14:12", preset: "Console Balanced", notes: "First scan, no writes", artifacts: ["CSV", "JSON", "HTML"] },
  { id: "run_02", date: "2025-09-11 09:18", preset: "Console Balanced", notes: "Applied textures quick fixes", artifacts: ["CSV", "JSON", "HTML"] },
  { id: "run_03", date: "2025-09-12 17:46", preset: "VR Crisp UI", notes: "UI pass", artifacts: ["CSV", "JSON", "HTML"] },
];

const leftTree = [
  { id: "game", label: "Game", children: [
      { id: "maps", label: "Maps" },
      { id: "meshes", label: "Meshes" },
      { id: "textures", label: "Textures" },
      { id: "materials", label: "Materials" },
      { id: "ui", label: "UI" },
      { id: "audio", label: "Audio" },
      { id: "niagara", label: "Niagara" },
    ] },
];

const TEXTURE_GROUPS = [
  { id: "World", maxSize: 4096, lodBias: 0 },
  { id: "Characters", maxSize: 4096, lodBias: 0 },
  { id: "Props", maxSize: 2048, lodBias: 0 },
  { id: "UI", maxSize: 2048, lodBias: 0 },
  { id: "Effects", maxSize: 1024, lodBias: 1 },
];

// ---------- Root App ----------
export default function App() {
  const [tab, setTab] = useState("audit");
  const [preset, setPreset] = useState(PRESETS[1]);
  const [targets, setTargets] = useState(["ps5", "xbox"]);
  const [scopeSummary, setScopeSummary] = useState("/Game, 2 excludes");
  const [openScope, setOpenScope] = useState(false);
  const [openPlan, setOpenPlan] = useState(false);
  const [openDrawer, setOpenDrawer] = useState(false);
  const [search, setSearch] = useState("");
  const [changeCap, setChangeCap] = useState(800);
  const [selectedRows, setSelectedRows] = useState([]);
  const [slideOverRow, setSlideOverRow] = useState(null);
  const [runProgress, setRunProgress] = useState({ running: false, pct: 0, msg: "" });
  const [taskList, setTaskList] = useState([]);
  const [showPalette, setShowPalette] = useState(false);

  // Keyboard shortcuts: Ctrl+Shift+P opens command palette
  useEffect(() => {
    const onKey = (e) => {
      if ((e.ctrlKey || e.metaKey) && e.shiftKey && e.key.toLowerCase() === "p") {
        e.preventDefault();
        setShowPalette(true);
      }
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, []);

  const filteredTextures = useMemo(() => {
    if (!search) return MOCK_TEXTURES;
    const s = search.toLowerCase();
    return MOCK_TEXTURES.filter(r => r.path.toLowerCase().includes(s) || r.group.toLowerCase().includes(s) || r.compression.toLowerCase().includes(s));
  }, [search]);

  function toggleTarget(id) {
    setTargets(prev => prev.includes(id) ? prev.filter(t => t !== id) : [...prev, id]);
  }

  function startScan({ autoFix = false, preview = false }) {
    if (preview) { setOpenPlan(true); return; }
    setRunProgress({ running: true, pct: 0, msg: autoFix ? "Auto fixing…" : "Scanning…" });
    const id = `task_${Date.now()}`;
    setTaskList(t => [{ id, label: autoFix ? "Auto Fix" : "Scan", status: "Running", pct: 0 }, ...t]);
    // Fake progress
    let pct = 0;
    const h = setInterval(() => {
      pct += Math.random() * 12;
      if (pct >= 100) {
        clearInterval(h);
        setRunProgress({ running: false, pct: 100, msg: autoFix ? "Auto fix done" : "Scan complete" });
        setTaskList(t => t.map(task => task.id === id ? { ...task, status: "Done", pct: 100 } : task));
      } else {
        setRunProgress({ running: true, pct, msg: autoFix ? "Auto fixing…" : "Scanning…" });
        setTaskList(t => t.map(task => task.id === id ? { ...task, pct } : task));
      }
    }, 350);
  }

  const selectedObjs = filteredTextures.filter(r => selectedRows.includes(r.id));
  const proposed = useMemo(() => buildProposedChanges(selectedObjs), [selectedObjs]);
  const playbook = inferPlaybook(filteredTextures);

  return (
    <TooltipProvider>
      <div className="w-full h-full bg-background text-foreground font-sans">
        <Header
          preset={preset}
          setPreset={setPreset}
          targets={targets}
          toggleTarget={toggleTarget}
          scopeSummary={scopeSummary}
          onOpenScope={() => setOpenScope(true)}
          changeCap={changeCap}
          setChangeCap={setChangeCap}
          onRunScan={() => startScan({})}
          onAutoFix={() => startScan({ autoFix: true })}
          onPreview={() => startScan({ preview: true })}
          openDrawer={() => setOpenDrawer(true)}
          run={runProgress}
        />

        <div className="grid grid-cols-[260px_1fr_320px] gap-3 px-4 pb-4">
          <LeftRail />

          <main className="min-h-[72vh]">
            <Tabs value={tab} onValueChange={setTab} className="w-full">
              <TabsList className="sticky top-0 z-10 bg-muted/60 backdrop-blur supports-[backdrop-filter]:bg-muted/60">
                <TabsTrigger value="audit">Audit</TabsTrigger>
                <TabsTrigger value="budgets">Budgets</TabsTrigger>
                <TabsTrigger value="recommend">Recommend</TabsTrigger>
                <TabsTrigger value="apply">Apply</TabsTrigger>
                <TabsTrigger value="verify">Verify</TabsTrigger>
                <TabsTrigger value="reports">Reports</TabsTrigger>
              </TabsList>

              <TabsContent value="audit" className="mt-3">
                <AuditView
                  search={search}
                  setSearch={setSearch}
                  textures={filteredTextures}
                  selectedRows={selectedRows}
                  setSelectedRows={setSelectedRows}
                  onOpenRow={setSlideOverRow}
                />
              </TabsContent>

              <TabsContent value="budgets" className="mt-3">
                <BudgetsView textures={filteredTextures} />
              </TabsContent>

              <TabsContent value="recommend" className="mt-3">
                <RecommendView proposed={proposed} playbook={playbook} />
              </TabsContent>

              <TabsContent value="apply" className="mt-3">
                <ApplyView tasks={taskList} pause={() => { }} />
              </TabsContent>

              <TabsContent value="verify" className="mt-3">
                <VerifyView />
              </TabsContent>

              <TabsContent value="reports" className="mt-3">
                <ReportsView runs={mockRuns} />
              </TabsContent>
            </Tabs>
          </main>

          <RightShelf selectionCount={selectedRows.length} proposed={proposed} onPreview={() => setOpenPlan(true)} />
        </div>

        <ScopeSheet open={openScope} setOpen={setOpenScope} onApply={(summary) => setScopeSummary(summary)} />
        <PlanDialog open={openPlan} setOpen={setOpenPlan} proposed={proposed} changeCap={changeCap} />
        <SlideOver row={slideOverRow} onOpenChange={setSlideOverRow} targets={targets} />
        <TaskDrawer open={openDrawer} setOpen={setOpenDrawer} tasks={taskList} />
        <CommandPalette open={showPalette} setOpen={setShowPalette} onRun={() => startScan({})} onAutoFix={() => startScan({ autoFix: true })} />
      </div>
    </TooltipProvider>
  );
}

// ---------- Header ----------
function Header({ preset, setPreset, targets, toggleTarget, scopeSummary, onOpenScope, changeCap, setChangeCap, onRunScan, onAutoFix, onPreview, openDrawer, run }) {
  return (
    <div className="sticky top-0 z-20 w-full border-b bg-card/80 backdrop-blur supports-[backdrop-filter]:bg-card/80">
      <div className="mx-auto grid grid-cols-[260px_1fr_320px] gap-3 px-4 py-2">
        <div className="flex items-center gap-2">
          <div className="text-lg font-semibold">MagicOptimizer</div>
        </div>
        <div className="flex items-center gap-2 flex-wrap">
          {/* Preset */}
          <DropdownMenu>
            <DropdownMenuTrigger asChild>
              <Button variant="outline" size="sm" className="min-w-[160px] justify-between">Preset: {preset}</Button>
            </DropdownMenuTrigger>
            <DropdownMenuContent align="start" className="w-56">
              {PRESETS.map((p) => (
                <DropdownMenuItem key={p} onClick={() => setPreset(p)} className="justify-between">
                  {p} {p === preset ? <Badge>Current</Badge> : null}
                </DropdownMenuItem>
              ))}
            </DropdownMenuContent>
          </DropdownMenu>
          {/* Targets */}
          <div className="flex items-center gap-1 rounded-md border px-2 py-1.5">
            {TARGET_CHIPS.map((t) => (
              <Button key={t.id} size="icon" variant={targets.includes(t.id) ? "default" : "ghost"} className="h-8 w-8" onClick={() => toggleTarget(t.id)} title={t.label}>
                {t.icon}
              </Button>
            ))}
          </div>
          {/* Scope */}
          <Button variant="outline" size="sm" onClick={onOpenScope}>Scope: {scopeSummary}</Button>
          {/* Cap */}
          <div className="hidden md:flex items-center gap-2 border rounded-md px-2 py-1.5">
            <Label htmlFor="cap" className="text-xs">Change Cap</Label>
            <Input id="cap" value={changeCap} onChange={(e) => setChangeCap(Number(e.target.value) || 0)} className="h-8 w-20" />
          </div>
        </div>
        <div className="flex items-center justify-end gap-2">
          <Button onClick={onRunScan} className="gap-2"><Play className="h-4 w-4" /> Run Scan</Button>
          <Button variant="secondary" onClick={onAutoFix} className="gap-2"><Wand2 className="h-4 w-4" /> Auto Fix</Button>
          <Button variant="outline" onClick={onPreview} className="gap-2"><Eye className="h-4 w-4" /> Preview</Button>
          <Button variant="ghost" onClick={openDrawer} className="gap-2"><ClipboardList className="h-4 w-4" /> Tasks</Button>
        </div>
      </div>
      {/* progress bar */}
      <AnimatePresence>
        {run.running && (
          <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }} className="w-full">
            <div className="px-4 pb-2">
              <div className="flex items-center gap-3">
                <div className="text-xs text-muted-foreground min-w-[80px]">{run.msg}</div>
                <Progress value={run.pct} className="h-2" />
                <div className="text-xs w-10 text-right">{Math.round(run.pct)}%</div>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}

// ---------- Left Rail ----------
function LeftRail() {
  return (
    <aside className="h-[calc(100vh-110px)] sticky top-[110px] rounded-xl border p-2">
      <ScrollArea className="h-full pr-2">
        <div className="px-1">
          <div className="font-medium text-sm mb-2">Project Scope</div>
          <Tree items={leftTree} />
          <Separator className="my-3" />
          <div className="font-medium text-sm mb-2">Asset Types</div>
          {["Textures", "Meshes", "Materials", "Blueprints", "Animations", "Audio", "Niagara", "Levels", "UI"].map(k => (
            <label key={k} className="flex items-center gap-2 py-1 text-sm"><Checkbox defaultChecked={k === "Textures"} /> {k}</label>
          ))}
          <Separator className="my-3" />
          <div className="font-medium text-sm mb-2">Saved Views</div>
          <div className="space-y-1">
            <Button variant="ghost" size="sm" className="w-full justify-start">High Impact</Button>
            <Button variant="ghost" size="sm" className="w-full justify-start">UI Pass</Button>
            <Button variant="ghost" size="sm" className="w-full justify-start">Console Ready</Button>
          </div>
        </div>
      </ScrollArea>
    </aside>
  );
}

function Tree({ items }) {
  return <div className="space-y-1">{items.map((n) => <TreeNode key={n.id} node={n} depth={0} />)}</div>;
}
function TreeNode({ node, depth }) {
  const [open, setOpen] = useState(true);
  return (
    <div>
      <div className="flex items-center gap-1 pl-1 py-0.5">
        {node.children && (<Button variant="ghost" size="icon" className="h-6 w-6" onClick={() => setOpen(o => !o)}>{open ? <ChevronDown className="h-4 w-4" /> : <ChevronRight className="h-4 w-4" />}</Button>)}
        <Checkbox defaultChecked className="mr-2" />
        <span className="text-sm">{node.label}</span>
      </div>
      {open && node.children && (
        <div className="pl-6">
          {node.children.map((c) => (
            <div key={c.id} className="flex items-center gap-2 py-0.5"><Checkbox defaultChecked={c.id !== "audio"} /><span className="text-sm">{c.label}</span></div>
          ))}
        </div>
      )}
    </div>
  );
}

// ---------- Audit View ----------
function AuditView({ search, setSearch, textures, selectedRows, setSelectedRows, onOpenRow }) {
  const [severityFilter, setSeverityFilter] = useState(null);

  const rows = useMemo(() => {
    if (!severityFilter) return textures;
    if (severityFilter === "red") return textures.filter((r) => r.issues >= 2);
    if (severityFilter === "amber") return textures.filter((r) => r.issues === 1);
    return textures.filter((r) => r.issues === 0);
  }, [textures, severityFilter]);

  return (
    <div className="space-y-3">
      <div className="grid grid-cols-3 gap-3">
        <Card className="col-span-1">
          <CardHeader className="pb-2"><CardTitle className="text-sm">Health</CardTitle></CardHeader>
          <CardContent className="pt-0">
            <div className="text-3xl font-semibold">{78}<span className="text-muted-foreground text-base">/100</span></div>
            <div className="h-24">
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={healthHistory}>
                  <CartesianGrid strokeDasharray="3 3" />
                  <XAxis dataKey="t" hide /><YAxis hide /><RTooltip />
                  <Line type="monotone" dataKey="score" strokeWidth={2} dot={false} />
                </LineChart>
              </ResponsiveContainer>
            </div>
          </CardContent>
        </Card>
        <Card className="col-span-1">
          <CardHeader className="pb-2"><CardTitle className="text-sm">Top Offenders</CardTitle></CardHeader>
          <CardContent className="pt-0 text-sm space-y-1">
            <div className="flex items-center justify-between"><span>Masks with sRGB on</span><Badge variant="destructive">45</Badge></div>
            <div className="flex items-center justify-between"><span>Normals not BC5</span><Badge variant="secondary">82</Badge></div>
            <div className="flex items-center justify-between"><span>Non VT assets flagged VT</span><Badge>16</Badge></div>
          </CardContent>
        </Card>
        <Card className="col-span-1">
          <CardHeader className="pb-2"><CardTitle className="text-sm">Summary</CardTitle></CardHeader>
          <CardContent className="pt-0 text-sm space-y-1">
            <div>Preset <Badge variant="outline">Console Balanced</Badge></div>
            <div>Targets <Badge>PS5</Badge> <Badge>Xbox</Badge></div>
            <div>Scope <Badge variant="outline">/Game</Badge> <Badge variant="outline">2 excludes</Badge></div>
          </CardContent>
        </Card>
      </div>

      <Card>
        <CardHeader className="pb-2">
          <div className="flex items-center justify-between">
            <CardTitle className="text-sm">Textures</CardTitle>
            <div className="flex items-center gap-2">
              <div className="relative">
                <SearchIcon className="absolute left-2 top-2.5 h-4 w-4 text-muted-foreground" />
                <Input placeholder="Search" value={search} onChange={e => setSearch(e.target.value)} className="pl-7 w-[220px]" />
              </div>
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button variant="outline" size="sm" className="gap-2"><Filter className="h-4 w-4" /> Severity</Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="end">
                  <DropdownMenuCheckboxItem checked={severityFilter === null} onCheckedChange={() => setSeverityFilter(null)}>All</DropdownMenuCheckboxItem>
                  <DropdownMenuCheckboxItem checked={severityFilter === "red"} onCheckedChange={() => setSeverityFilter("red")}>Must fix</DropdownMenuCheckboxItem>
                  <DropdownMenuCheckboxItem checked={severityFilter === "amber"} onCheckedChange={() => setSeverityFilter("amber")}>Should fix</DropdownMenuCheckboxItem>
                  <DropdownMenuCheckboxItem checked={severityFilter === "blue"} onCheckedChange={() => setSeverityFilter("blue")}>Nice to fix</DropdownMenuCheckboxItem>
                </DropdownMenuContent>
              </DropdownMenu>
            </div>
          </div>
        </CardHeader>
        <CardContent className="pt-0">
          <div className="rounded-md border overflow-hidden">
            <Table>
              <TableHeader>
                <TableRow>
                  <TableHead className="w-10"></TableHead>
                  <TableHead>Path</TableHead>
                  <TableHead className="w-16">Size</TableHead>
                  <TableHead className="w-28">Group</TableHead>
                  <TableHead className="w-36">Compression</TableHead>
                  <TableHead className="w-16">sRGB</TableHead>
                  <TableHead className="w-16">Mips</TableHead>
                  <TableHead className="w-16">VT</TableHead>
                  <TableHead className="w-16">LOD</TableHead>
                  <TableHead className="w-20">Refs</TableHead>
                  <TableHead className="w-24">Issues</TableHead>
                  <TableHead className="w-48">Actions</TableHead>
                </TableRow>
              </TableHeader>
              <TableBody>
                {rows.map((r) => {
                  const checked = selectedRows.includes(r.id);
                  return (
                    <TableRow key={r.id} className={checked ? "bg-muted/40" : undefined}>
                      <TableCell className="w-10">
                        <Checkbox checked={checked} onCheckedChange={(v) => { setSelectedRows(prev => v ? [...prev, r.id] : prev.filter(id => id !== r.id)); }} />
                      </TableCell>
                      <TableCell className="font-mono text-xs">{r.path}</TableCell>
                      <TableCell>{r.size}</TableCell>
                      <TableCell>{r.group}</TableCell>
                      <TableCell>{r.compression}</TableCell>
                      <TableCell>{r.sRGB ? "On" : "Off"}</TableCell>
                      <TableCell>{r.mips ? "Yes" : "No"}</TableCell>
                      <TableCell>{r.vt ? "VT" : "—"}</TableCell>
                      <TableCell>{r.lod}</TableCell>
                      <TableCell>{r.referencers}</TableCell>
                      <TableCell><IssueBadges n={r.issues} /></TableCell>
                      <TableCell>
                        <div className="flex gap-2 flex-wrap">
                          <Button size="sm" variant="secondary" onClick={() => onOpenRow(r)}>Preview</Button>
                          <Button size="sm" variant="outline">Open</Button>
                          <Button size="sm" variant="outline">Open in Asset Audit</Button>
                          <Button size="sm" variant="outline">Open Size Map</Button>
                        </div>
                      </TableCell>
                    </TableRow>
                  );
                })}
              </TableBody>
            </Table>
          </div>
          <div className="flex items-center justify-between text-xs text-muted-foreground mt-2">
            <div>{rows.length} rows • {selectedRows.length} selected</div>
            <div>Scanning Textures… <span className="ml-1">6124 of 16214</span></div>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

function IssueBadges({ n }) {
  if (n >= 2) return <Badge variant="destructive">Must fix</Badge>;
  if (n === 1) return <Badge variant="secondary">Should fix</Badge>;
  return <Badge variant="outline">Info</Badge>;
}

// ---------- Budgets View ----------
function BudgetsView({ textures }) {
  // Mock calculations
  const poolUse = 86; // %
  const vtUse = 72; // %
  const diskUse = 64; // % against a target

  const offenders = useMemo(() => {
    const grouped = {};
    textures.forEach((t) => { grouped[t.group] = (grouped[t.group] || 0) + (t.size === "4k" ? 4 : t.size === "2k" ? 2 : 1); });
    return Object.entries(grouped).map(([group, weight]) => ({ group, weight })).sort((a, b) => b.weight - a.weight);
  }, [textures]);

  return (
    <div className="grid grid-cols-3 gap-3">
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm flex items-center gap-2"><Gauge className="h-4 w-4" /> Texture Pool</CardTitle></CardHeader>
        <CardContent className="pt-0 flex flex-col items-center gap-2">
          <Radial value={poolUse} />
          <div className="text-xs text-muted-foreground">Usage of streaming pool</div>
        </CardContent>
      </Card>
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm flex items-center gap-2"><Activity className="h-4 w-4" /> VT Pool</CardTitle></CardHeader>
        <CardContent className="pt-0 flex flex-col items-center gap-2">
          <Radial value={vtUse} />
          <div className="text-xs text-muted-foreground">Virtual texturing residency</div>
        </CardContent>
      </Card>
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm flex items-center gap-2"><HardDrive className="h-4 w-4" /> Disk Footprint</CardTitle></CardHeader>
        <CardContent className="pt-0 flex flex-col items-center gap-2">
          <Radial value={diskUse} />
          <div className="text-xs text-muted-foreground">Cooked content vs target</div>
        </CardContent>
      </Card>

      <Card className="col-span-3">
        <CardHeader className="pb-2"><CardTitle className="text-sm">Top Offenders by TextureGroup</CardTitle></CardHeader>
        <CardContent className="pt-0">
          <div className="rounded-md border overflow-hidden">
            <Table>
              <TableHeader>
                <TableRow>
                  <TableHead>Group</TableHead>
                  <TableHead className="w-64">Weighted Cost</TableHead>
                  <TableHead className="w-64">Suggested Action</TableHead>
                </TableRow>
              </TableHeader>
              <TableBody>
                {offenders.map((o) => (
                  <TableRow key={o.group}>
                    <TableCell>{o.group}</TableCell>
                    <TableCell>
                      <Progress value={Math.min(o.weight * 8, 100)} className="h-2" />
                    </TableCell>
                    <TableCell>
                      {o.group === "UI" ? "Verify crispness / consider 2k cap" : "Lower MaxTextureSize or increase LODBias"}
                    </TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

function Radial({ value }) {
  const data = [{ name: "u", value }];
  return (
    <ResponsiveContainer width={160} height={160}>
      <RadialBarChart cx="50%" cy="50%" innerRadius={60} outerRadius={80} barSize={14} data={data} startAngle={90} endAngle={-270}>
        <RadialBar background dataKey="value" cornerRadius={8} />
        <PolarAngleAxis type="number" domain={[0, 100]} tick={false} />
        <RTooltip />
      </RadialBarChart>
    </ResponsiveContainer>
  );
}

// ---------- Recommend View ----------
function buildProposedChanges(rows) {
  const masksSRGB = rows.filter(r => r.compression === "Masks" && r.sRGB);
  const normalsBC5 = rows.filter(r => r.compression === "Normal");
  const vtOff = rows.filter(r => !r.vt && r.group !== "World");
  return [
    { id: "srgb_off", label: `Set sRGB off for ${masksSRGB.length} mask textures`, items: masksSRGB },
    { id: "normals_bc5", label: `Convert ${normalsBC5.length} normals to BC5`, items: normalsBC5 },
    { id: "disable_vt", label: `Disable VT on ${vtOff.length} non VT assets`, items: vtOff },
  ];
}

function inferPlaybook(textures) {
  const big = textures.filter(t => t.size === "4k").length;
  if (big > 20) return { title: "Streaming Pool Risk", body: "Large number of 4k textures detected. Consider lowering MaxTextureSize or increasing LODBias for World/Props groups.", actions: ["Open Budgets", "Filter 4k", "Preview Plan"] };
  return null;
}

function RecommendView({ proposed, playbook }) {
  const estDisk = 180 + (proposed[0]?.items.length || 0) * 2;
  const estMem = 320 + (proposed[1]?.items.length || 0) * 1.2;
  return (
    <div className="space-y-3">
      {playbook && (
        <Card className="border-amber-500/40">
          <CardHeader className="pb-1"><CardTitle className="text-sm">Playbook: {playbook.title}</CardTitle></CardHeader>
          <CardContent className="pt-0 text-sm flex items-center justify-between">
            <div>{playbook.body}</div>
            <div className="flex gap-2">
              {playbook.actions.map((a) => <Button key={a} variant="outline" size="sm">{a}</Button>)}
            </div>
          </CardContent>
        </Card>
      )}
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Summary</CardTitle></CardHeader>
        <CardContent className="pt-0 text-sm grid grid-cols-3 gap-3">
          <div>Estimated disk saved <div className="text-2xl font-semibold">{Math.round(estDisk)} MB</div></div>
          <div>Estimated memory saved <div className="text-2xl font-semibold">{Math.round(estMem)} MB</div></div>
          <div>Risk notes <div className="text-xs text-muted-foreground">Check UI assets for crispness before commit</div></div>
        </CardContent>
      </Card>
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Proposed changes</CardTitle></CardHeader>
        <CardContent className="pt-0 space-y-2">
          {proposed.map((p) => (
            <div key={p.id} className="flex items-center justify-between rounded-md border p-2">
              <div className="text-sm">{p.label}</div>
              <div className="flex items-center gap-2">
                <Badge variant="outline">{p.items.length}</Badge>
                <Button size="sm" variant="outline">Show details</Button>
              </div>
            </div>
          ))}
          <div className="pt-2 flex gap-2">
            <Button className="gap-2"><Eye className="h-4 w-4" /> Preview Plan</Button>
            <Button variant="secondary" className="gap-2"><Wand2 className="h-4 w-4" /> Apply</Button>
            <Button variant="outline" className="gap-2"><FileDown className="h-4 w-4" /> Copy commandlet</Button>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

// ---------- Apply View ----------
function ApplyView({ tasks }) {
  return (
    <Card>
      <CardHeader className="pb-2"><CardTitle className="text-sm">Queue</CardTitle></CardHeader>
      <CardContent className="pt-0 space-y-3">
        {tasks.length === 0 && <div className="text-sm text-muted-foreground">No active tasks.</div>}
        {tasks.map((t) => (
          <div key={t.id} className="rounded-md border p-2">
            <div className="flex items-center justify-between text-sm"><div className="font-medium">{t.label}</div><Badge variant={t.status === "Done" ? "default" : "secondary"}>{t.status}</Badge></div>
            <Progress value={t.pct} className="h-2 mt-2" />
          </div>
        ))}
        <div className="text-xs text-muted-foreground">When done, copy this commit message: <span className="font-mono">MagicOptimizer apply {"{preset}"} on selected textures</span></div>
      </CardContent>
    </Card>
  );
}

// ---------- Verify View ----------
function VerifyView() {
  return (
    <div className="grid grid-cols-2 gap-3">
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Before and After</CardTitle></CardHeader>
        <CardContent className="pt-0 h-64">
          <ResponsiveContainer width="100%" height="100%">
            <BarChart data={beforeAfter}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="name" />
              <YAxis />
              <Legend />
              <RTooltip />
              <Bar dataKey="Before" fillOpacity={0.7} />
              <Bar dataKey="After" fillOpacity={0.7} />
            </BarChart>
          </ResponsiveContainer>
        </CardContent>
      </Card>
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Regressions</CardTitle></CardHeader>
        <CardContent className="pt-0"><div className="text-sm text-muted-foreground">No regressions detected.</div></CardContent>
      </Card>
    </div>
  );
}

// ---------- Reports View ----------
function ReportsView({ runs }) {
  return (
    <div className="grid grid-cols-[320px_1fr] gap-3">
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Run History</CardTitle></CardHeader>
        <CardContent className="pt-0">
          <div className="space-y-2">
            {runs.map((r) => (
              <div key={r.id} className="rounded-md border p-2"><div className="text-sm font-medium">{r.date}</div><div className="text-xs text-muted-foreground">{r.preset} • {r.notes}</div></div>
            ))}
          </div>
        </CardContent>
      </Card>
      <Card>
        <CardHeader className="pb-2"><CardTitle className="text-sm">Artifacts</CardTitle></CardHeader>
        <CardContent className="pt-0 flex gap-2 flex-wrap">
          <Button variant="outline" className="gap-2"><FileDown className="h-4 w-4" /> CSV</Button>
          <Button variant="outline" className="gap-2"><FileDown className="h-4 w-4" /> JSON</Button>
          <Button variant="outline" className="gap-2"><FileDown className="h-4 w-4" /> HTML</Button>
          <Button variant="ghost" className="gap-2"><Link2 className="h-4 w-4" /> Copy Share Link</Button>
        </CardContent>
      </Card>
    </div>
  );
}

// ---------- Right Shelf ----------
function RightShelf({ selectionCount, proposed, onPreview }) {
  return (
    <aside className="h-[calc(100vh-110px)] sticky top-[110px] rounded-xl border p-2">
      <div className="px-1">
        <div className="font-medium text-sm mb-2">Quick Fixes</div>
        {proposed.map((p) => (<Button key={p.id} variant="outline" className="w-full justify-start mb-2">{p.label}</Button>))}
        <Button className="w-full mt-2 gap-2" onClick={onPreview}><Eye className="h-4 w-4" /> Preview Plan</Button>
        <Separator className="my-3" />
        <div className="font-medium text-sm mb-2">Bridges</div>
        <div className="grid gap-2">
          <Button variant="ghost" className="justify-start">Open in Asset Audit</Button>
          <Button variant="ghost" className="justify-start">Open Size Map</Button>
          <Button variant="ghost" className="justify-start">Start 30s Insights Trace</Button>
        </div>
        <Separator className="my-3" />
        <div className="text-xs text-muted-foreground">Selection {selectionCount || 0}</div>
        <div className="text-xs text-muted-foreground">Docs <a className="underline" href="#">Open</a></div>
      </div>
    </aside>
  );
}

// ---------- Scope Sheet (with Profiles & Groups) ----------
function ScopeSheet({ open, setOpen, onApply }) {
  const [tab, setTab] = useState("include");
  const [folders, setFolders] = useState({ game: true, maps: true, ui: true, materials: true, audio: false });
  const [exclude, setExclude] = useState(["/Game/Developer"]);
  const [applyOnly, setApplyOnly] = useState(true);
  const [groups, setGroups] = useState(TEXTURE_GROUPS);

  return (
    <Sheet open={open} onOpenChange={setOpen}>
      <SheetContent side="top" className="max-w-5xl mx-auto">
        <SheetHeader>
          <SheetTitle>Scope & Profiles</SheetTitle>
          <SheetDescription>Choose scope and tweak TextureGroup budgets safely.</SheetDescription>
        </SheetHeader>
        <Tabs value={tab} onValueChange={setTab} className="pt-2">
          <TabsList>
            <TabsTrigger value="include">Include / Exclude</TabsTrigger>
            <TabsTrigger value="profiles">Profiles & Groups</TabsTrigger>
          </TabsList>
          <TabsContent value="include" className="pt-3">
            <div className="grid grid-cols-2 gap-6">
              <div>
                <div className="font-medium mb-2">Include</div>
                {Object.entries(folders).map(([k, v]) => (
                  <label key={k} className="flex items-center gap-2 py-1"><Checkbox checked={!!v} onCheckedChange={(val) => setFolders(prev => ({ ...prev, [k]: !!val }))} /><span className="text-sm">{k}</span></label>
                ))}
              </div>
              <div>
                <div className="font-medium mb-2">Excludes</div>
                <div className="space-y-2">
                  {exclude.map((e, i) => (
                    <div key={i} className="flex items-center gap-2"><Input value={e} onChange={(ev) => setExclude(curr => curr.map((c, ii) => ii === i ? ev.target.value : c))} /><Button variant="ghost" size="icon" onClick={() => setExclude(curr => curr.filter((_, ii) => ii !== i))}><X className="h-4 w-4" /></Button></div>
                  ))}
                  <Button variant="outline" size="sm" onClick={() => setExclude(curr => [...curr, "/Game/Unused"]) }>Add exclude</Button>
                </div>
                <Separator className="my-3" />
                <label className="flex items-center gap-2"><Switch checked={applyOnly} onCheckedChange={setApplyOnly} /> Apply Only on assets with clear classification</label>
              </div>
            </div>
          </TabsContent>
          <TabsContent value="profiles" className="pt-3">
            <div className="grid grid-cols-2 gap-6">
              <div className="space-y-2">
                {groups.map((g, idx) => (
                  <Card key={g.id}>
                    <CardHeader className="pb-2"><CardTitle className="text-sm">{g.id}</CardTitle></CardHeader>
                    <CardContent className="pt-0 flex items-center gap-3">
                      <Label className="text-xs">MaxTextureSize</Label>
                      <Input className="w-24 h-8" type="number" value={g.maxSize} onChange={(e) => setGroups(list => list.map((x, i) => i === idx ? { ...x, maxSize: parseInt(e.target.value||"0") } : x))} />
                      <Label className="text-xs">LODBias</Label>
                      <Input className="w-16 h-8" type="number" value={g.lodBias} onChange={(e) => setGroups(list => list.map((x, i) => i === idx ? { ...x, lodBias: parseInt(e.target.value||"0") } : x))} />
                    </CardContent>
                  </Card>
                ))}
              </div>
              <div>
                <Card>
                  <CardHeader className="pb-2"><CardTitle className="text-sm">Preview .ini Diff</CardTitle></CardHeader>
                  <CardContent className="pt-0">
                    <pre className="bg-muted p-3 rounded-md text-xs whitespace-pre-wrap">{renderIni(groups)}</pre>
                  </CardContent>
                </Card>
              </div>
            </div>
          </TabsContent>
        </Tabs>
        <SheetFooter>
          <Button onClick={() => { setOpen(false); onApply("/Game, 2 excludes"); }}>Apply</Button>
        </SheetFooter>
      </SheetContent>
    </Sheet>
  );
}

function renderIni(groups) {
  let lines = ["[/Script/Engine.TextureLODSettings]", "TextureLODGroups=(
" ];
  lines.push(...groups.map(g => `  (${g.id} MaxTextureSize=${g.maxSize}, LODBias=${g.lodBias})`));
  lines.push(")
" );
  return lines.join("
");
}

// ---------- Plan Dialog ----------
function PlanDialog({ open, setOpen, proposed, changeCap }) {
  const total = proposed.reduce((acc, p) => acc + p.items.length, 0);
  const overCap = total > changeCap;
  return (
    <Dialog open={open} onOpenChange={setOpen}>
      <DialogContent className="max-w-2xl">
        <DialogHeader><DialogTitle>Preview Plan</DialogTitle><DialogDescription>Review what would change before applying.</DialogDescription></DialogHeader>
        <div className="space-y-2">
          {proposed.map((p) => (<div key={p.id} className="rounded-md border p-2"><div className="font-medium text-sm">{p.label}</div><div className="text-xs text-muted-foreground">Sample: {p.items.slice(0, 3).map((i) => i.path).join(", ")} …</div></div>))}
          <Separator />
          <div className="text-sm">Total changes <Badge variant={overCap ? "destructive" : "secondary"}>{total}</Badge> Cap <Badge variant="outline">{changeCap}</Badge></div>
          {overCap && <div className="text-xs text-destructive">Plan exceeds cap. Confirmation required at apply.</div>}
        </div>
        <DialogFooter>
          <Button variant="secondary">Apply</Button>
          <Button variant="outline" onClick={() => setOpen(false)}>Close</Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
}

// ---------- Slide Over Row Preview with What-if ----------
function SlideOver({ row, onOpenChange, targets }) {
  const [maxSize, setMaxSize] = useState(2048);
  const [lodBias, setLodBias] = useState(0);
  const [comp, setComp] = useState(null);
  useEffect(() => {
    if (row) { setMaxSize(row.size === "4k" ? 4096 : row.size === "2k" ? 2048 : 1024); setLodBias(row.lod || 0); setComp(row.compression); }
  }, [row]);

  if (!row) return null;
  const cooked = targets.map((t) => ({ target: t.toUpperCase(), format: cookedFormat(comp || row.compression, t) }));

  return (
    <AnimatePresence>
      {row && (
        <motion.div initial={{ x: 400, opacity: 0 }} animate={{ x: 0, opacity: 1 }} exit={{ x: 400, opacity: 0 }} transition={{ duration: 0.2 }} className="fixed right-0 top-[110px] h-[calc(100vh-110px)] w-[460px] border-l bg-background z-30 shadow-xl">
          <div className="p-3 flex items-center justify-between border-b"><div className="font-medium text-sm">{row.path}</div><Button variant="ghost" size="icon" onClick={() => onOpenChange(null)}><X className="h-4 w-4" /></Button></div>
          <div className="p-3 space-y-3 text-sm">
            <div className="grid grid-cols-2 gap-2">
              <div className="rounded-md border p-2"><div className="text-xs text-muted-foreground">Before</div><div className="mt-1">Compression {row.compression}</div><div>sRGB {row.sRGB ? "On" : "Off"}</div><div>Mips {row.mips ? "Yes" : "No"}</div></div>
              <div className="rounded-md border p-2"><div className="text-xs text-muted-foreground">After (What‑if)</div><div className="mt-1">Compression {comp}</div><div>MaxTextureSize {maxSize}</div><div>LODBias {lodBias}</div></div>
            </div>
            <Separator />
            <div className="grid grid-cols-3 gap-2">
              <div>
                <Label className="text-xs">MaxTextureSize</Label>
                <Input type="number" className="h-8" value={maxSize} onChange={(e) => setMaxSize(parseInt(e.target.value||"0"))} />
              </div>
              <div>
                <Label className="text-xs">LODBias</Label>
                <Input type="number" className="h-8" value={lodBias} onChange={(e) => setLodBias(parseInt(e.target.value||"0"))} />
              </div>
              <div>
                <Label className="text-xs">Compression</Label>
                <DropdownMenu>
                  <DropdownMenuTrigger asChild><Button variant="outline" className="w-full h-8 justify-between">{comp}</Button></DropdownMenuTrigger>
                  <DropdownMenuContent align="end" className="w-40">
                    {["Default","Masks","UserInterface","Normal","HDR","BC5","BC7"].map(c => <DropdownMenuItem key={c} onClick={() => setComp(c)}>{c}</DropdownMenuItem>)}
                  </DropdownMenuContent>
                </DropdownMenu>
              </div>
            </div>
            <div className="rounded-md border p-2">
              <div className="font-medium text-xs mb-1">Cooked formats by target</div>
              <div className="grid grid-cols-3 gap-1 text-xs">
                {cooked.map(c => (<div key={c.target} className="flex items-center justify-between"><span>{c.target}</span><Badge variant="outline">{c.format}</Badge></div>))}
              </div>
            </div>
            <div className="text-xs text-muted-foreground">Estimates: Disk −{Math.round((maxSize/1024)*12)} MB • VRAM −{Math.round((lodBias)*50)} MB (mock)</div>
            <div className="flex gap-2 pt-1">
              <Button size="sm" variant="secondary"><Wand2 className="h-4 w-4" /> Apply to this asset</Button>
              <Button size="sm" variant="outline" onClick={() => onOpenChange(null)}>Close</Button>
            </div>
          </div>
        </motion.div>
      )}
    </AnimatePresence>
  );
}

function cookedFormat(comp, target) {
  const isMobile = target === "quest" || target === "switch";
  if (comp === "Normal") return isMobile ? "ETC2/ASTC" : "BC5";
  if (comp === "HDR") return isMobile ? "ASTC HDR" : "BC6H";
  if (comp === "UserInterface") return isMobile ? "ETC2 RGBA" : "BC7";
  if (comp === "Masks") return isMobile ? "ETC2 RG" : "BC4/BC5";
  return isMobile ? "ETC2" : "BC7";
}

// ---------- Task Drawer ----------
function TaskDrawer({ open, setOpen, tasks }) {
  return (
    <Dialog open={open} onOpenChange={setOpen}>
      <DialogContent className="max-w-lg">
        <DialogHeader><DialogTitle>Tasks</DialogTitle><DialogDescription>Active and recent tasks</DialogDescription></DialogHeader>
        <div className="space-y-2">
          {tasks.length === 0 && <div className="text-sm text-muted-foreground">No tasks yet.</div>}
          {tasks.map((t) => (<div key={t.id} className="rounded-md border p-2"><div className="flex items-center justify-between text-sm"><div>{t.label}</div><Badge variant={t.status === "Done" ? "default" : "secondary"}>{t.status}</Badge></div><Progress value={t.pct} className="h-2 mt-2" /></div>))}
        </div>
        <DialogFooter><Button variant="outline" onClick={() => setOpen(false)}>Close</Button></DialogFooter>
      </DialogContent>
    </Dialog>
  );
}

// ---------- Command Palette ----------
function CommandPalette({ open, setOpen, onRun, onAutoFix }) {
  const ref = useRef(null);
  useEffect(() => { if (open) setTimeout(() => ref.current?.focus(), 50); }, [open]);
  return (
    <Dialog open={open} onOpenChange={setOpen}>
      <DialogContent className="max-w-xl">
        <DialogHeader><DialogTitle>Command Palette</DialogTitle><DialogDescription>Type to filter commands. Use Enter to run.</DialogDescription></DialogHeader>
        <div className="relative"><SearchIcon className="absolute left-2 top-2.5 h-4 w-4 text-muted-foreground" /><Input ref={ref} placeholder="Run Scan, Auto Fix, Export CSV, Jump to Textures…" className="pl-7" /></div>
        <div className="grid grid-cols-2 gap-2 pt-2">
          <Button className="justify-start" onClick={() => { onRun(); setOpen(false); }}><Play className="h-4 w-4 mr-2" /> Run Scan</Button>
          <Button className="justify-start" variant="secondary" onClick={() => { onAutoFix(); setOpen(false); }}><Wand2 className="h-4 w-4 mr-2" /> Auto Fix</Button>
          <Button className="justify-start" variant="outline"><FileDown className="h-4 w-4 mr-2" /> Copy commandlet</Button>
          <Button className="justify-start" variant="outline"><ClipboardCheck className="h-4 w-4 mr-2" /> Toggle Rules</Button>
        </div>
      </DialogContent>
    </Dialog>
  );
}
/* ==== CONTINUATION: Diagnostics, HUD, and Utilities (paste directly after the previous line) ==== */

// Diagnostics: Nanite, Landscape, and VT triage cards
export function DiagnosticsView() {
  const naniteMeshes = React.useMemo(
    () => [
      { path: "/Game/Env/SM_Rock_01", tris: 520000, mats: 3, warn: "Small triangles" },
      { path: "/Game/Props/SM_Chair_02", tris: 310000, mats: 1, warn: "High density vs screen" },
      { path: "/Game/Characters/SM_NPC_01", tris: 980000, mats: 5, warn: "Bounds precision" },
    ],
    []
  );

  const landscapes = React.useMemo(
    () => [
      { name: "LS_Desert", comps: 64, res: "4033×4033", warn: "Too many components" },
      { name: "LS_Forest", comps: 32, res: "2017×2017", warn: "OK" },
    ],
    []
  );

  const vt = { residency: 72, oversub: 18 };

  return (
    <div className="grid grid-cols-2 gap-3">
      <Card>
        <CardHeader className="pb-2">
          <CardTitle className="text-sm flex items-center gap-2">
            <ClipboardList className="h-4 w-4" /> Nanite sanity
          </CardTitle>
        </CardHeader>
        <CardContent className="pt-0 space-y-2 text-sm">
          <div className="rounded-md border p-2">
            <div className="grid grid-cols-[1fr_90px_70px_140px] gap-2 font-mono text-xs text-muted-foreground mb-1">
              <div>Path</div><div>Tris</div><div>Mats</div><div>Warning</div>
            </div>
            {naniteMeshes.map((m) => (
              <div key={m.path} className="grid grid-cols-[1fr_90px_70px_140px] gap-2 items-center py-1">
                <div className="font-mono text-xs truncate" title={m.path}>{m.path}</div>
                <div>{m.tris.toLocaleString()}</div>
                <div>{m.mats}</div>
                <div><Badge variant={m.warn === "OK" ? "outline" : "destructive"}>{m.warn}</Badge></div>
              </div>
            ))}
          </div>
          <div className="text-xs text-muted-foreground">
            Hints: target larger triangle size, check position precision, reduce material slots where possible.
          </div>
        </CardContent>
      </Card>

      <Card>
        <CardHeader className="pb-2">
          <CardTitle className="text-sm flex items-center gap-2">
            <HardDrive className="h-4 w-4" /> Landscape health
          </CardTitle>
        </CardHeader>
        <CardContent className="pt-0 space-y-2 text-sm">
          <div className="rounded-md border p-2">
            <div className="grid grid-cols-[1fr_90px_110px_140px] gap-2 font-mono text-xs text-muted-foreground mb-1">
              <div>Name</div><div>Comps</div><div>Resolution</div><div>Note</div>
            </div>
            {landscapes.map((l) => (
              <div key={l.name} className="grid grid-cols-[1fr_90px_110px_140px] gap-2 items-center py-1">
                <div className="font-mono text-xs truncate">{l.name}</div>
                <div>{l.comps}</div>
                <div>{l.res}</div>
                <div><Badge variant={l.warn === "OK" ? "outline" : "secondary"}>{l.warn}</Badge></div>
              </div>
            ))}
          </div>
          <div className="text-xs text-muted-foreground">
            Hints: prefer 63×63 component tiles; reduce section size; audit virtual textures for landscape layers.
          </div>
        </CardContent>
      </Card>

      <Card className="col-span-2">
        <CardHeader className="pb-2">
          <CardTitle className="text-sm flex items-center gap-2">
            <Activity className="h-4 w-4" /> VT triage
          </CardTitle>
        </CardHeader>
        <CardContent className="pt-0 grid grid-cols-[1fr_1fr] gap-4">
          <div>
            <div className="text-xs mb-1">Residency</div>
            <Progress value={vt.residency} className="h-2" />
            <div className="text-xs text-muted-foreground mt-1">{vt.residency}% resident</div>
          </div>
          <div>
            <div className="text-xs mb-1">Oversubscription</div>
            <Progress value={vt.oversub} className="h-2" />
            <div className="text-xs text-muted-foreground mt-1">{vt.oversub}% over budget</div>
          </div>
          <Separator className="col-span-2" />
          <div className="col-span-2 flex items-center gap-2">
            <Button variant="outline" size="sm"><Link2 className="h-4 w-4 mr-1" /> Open Size Map</Button>
            <Button variant="outline" size="sm"><Link2 className="h-4 w-4 mr-1" /> Open Asset Audit</Button>
            <Button variant="secondary" size="sm"><ClipboardCheck className="h-4 w-4 mr-1" /> Start 30s Insights Trace</Button>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

// Floating, toggleable Viewport Verify HUD (mock)
export function ViewportHUD() {
  const [visible, setVisible] = React.useState(true);
  const stats = { pool: 83, vt: 69, fps: 58, gpuMs: 12.6 };

  return (
    <div>
      <Button onClick={() => setVisible(v => !v)} className="fixed bottom-4 right-4 z-50" variant="secondary">
        <Eye className="h-4 w-4 mr-2" /> {visible ? "Hide HUD" : "Show HUD"}
      </Button>
      <AnimatePresence>
        {visible && (
          <motion.div
            initial={{ y: 30, opacity: 0 }}
            animate={{ y: 0, opacity: 1 }}
            exit={{ y: 30, opacity: 0 }}
            className="fixed top-4 right-4 z-40 w-[320px] rounded-lg border bg-background/80 backdrop-blur p-3 shadow-lg"
          >
            <div className="flex items-center justify-between mb-2">
              <div className="text-sm font-medium">Viewport Verify</div>
              <Badge variant="outline">mock</Badge>
            </div>
            <div className="space-y-2 text-sm">
              <div>
                <div className="flex items-center justify-between text-xs">
                  <span>Texture Pool</span><span>{stats.pool}%</span>
                </div>
                <Progress value={stats.pool} className="h-2" />
              </div>
              <div>
                <div className="flex items-center justify-between text-xs">
                  <span>VT Residency</span><span>{stats.vt}%</span>
                </div>
                <Progress value={stats.vt} className="h-2" />
              </div>
              <div className="grid grid-cols-2 gap-2 text-xs">
                <div className="rounded-md border p-2"><div>FPS</div><div className="text-lg font-semibold">{stats.fps}</div></div>
                <div className="rounded-md border p-2"><div>GPU ms</div><div className="text-lg font-semibold">{stats.gpuMs}</div></div>
              </div>
              <div className="flex items-center gap-2 pt-1">
                <Button size="sm" variant="outline"><ClipboardCheck className="h-4 w-4 mr-1" /> Trace 30s</Button>
                <Button size="sm" variant="outline">Copy report link</Button>
              </div>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}

// Utilities: commandlet builder (string only), version tag badge, compression help link
export function buildCommandlet(mode, preset, scope) {
  // mode: "audit" | "apply" | "verify"
  const base = "UE4Editor-Cmd.exe Project.uproject -run=MagicOptimizer";
  const args = [`-mode=${mode}`, `-preset=\"${preset}\"`, `-scope=\"${scope}\"`, "-report"];
  return `${base} ${args.join(" ")}`;
}

export function VersionTag({ label = "v0.9 mock" }) {
  return (
    <div className="inline-flex items-center gap-2 px-2 py-1 rounded-md border text-xs">
      <Gauge className="h-3.5 w-3.5" /> {label}
    </div>
  );
}

export function CompressionLearnLink({ kind }) {
  const map = {
    Default: "https://docs.unrealengine.com/",
    Masks: "https://docs.unrealengine.com/",
    UserInterface: "https://docs.unrealengine.com/",
    Normal: "https://docs.unrealengine.com/",
    HDR: "https://docs.unrealengine.com/",
    BC5: "https://docs.unrealengine.com/",
    BC7: "https://docs.unrealengine.com/",
  };
  const href = map[kind] || map.Default;
  return (
    <a href={href} className="inline-flex items-center gap-1 text-xs underline" target="_blank" rel="noreferrer">
      Learn <Link2 className="h-3 w-3" />
    </a>
  );
}

/* ==== END CONTINUATION ==== */
