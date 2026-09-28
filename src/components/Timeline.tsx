import { useState } from 'react';

const phases = [
  {
    id: 1,
    name: 'Planning & Management',
    weeks: 'Week 1',
    marks: 10,
    gate: null,
    color: 'from-blue-500 to-blue-600',
    activities: ['Feasibility study (technical, operational, economic, legal, schedule)', 'Work breakdown structure & Gantt chart', 'Risk register (6+ risks with mitigation)'],
    deliverable: 'Project management plan',
    icon: '📋',
  },
  {
    id: 2,
    name: 'Requirements Engineering',
    weeks: 'Weeks 1–2',
    marks: 14,
    gate: 'Gate 1',
    color: 'from-cyan-500 to-cyan-600',
    activities: ['Stakeholder analysis & conflict resolution', 'SRS (12+ functional, 6+ non-functional requirements)', 'Use case model with 3 fully dressed specifications'],
    deliverable: 'Stakeholder analysis, SRS, use case model',
    icon: '📝',
  },
  {
    id: 3,
    name: 'System Design',
    weeks: 'Weeks 2–3',
    marks: 24,
    gate: 'Gate 2',
    color: 'from-purple-500 to-purple-600',
    activities: ['Architecture (layered/MVC) with component diagram', 'Class diagram with 2+ design patterns', 'State machine, activity & sequence diagrams', 'ERD normalised to 3NF + SQL schema'],
    deliverable: 'Software design document',
    icon: '🏗️',
  },
  {
    id: 4,
    name: 'Implementation',
    weeks: 'Weeks 4–5',
    marks: 21,
    gate: null,
    color: 'from-green-500 to-green-600',
    activities: ['C++17 with role-based access control', 'SQLite with parameterised queries', 'Salted password hashing', 'Audit logging for mark changes', 'CMake build system'],
    deliverable: 'Source code in Git repository with build guide',
    icon: '💻',
  },
  {
    id: 5,
    name: 'Verification & Validation',
    weeks: 'Weeks 5–6',
    marks: 12,
    gate: 'Gate 3',
    color: 'from-amber-500 to-amber-600',
    activities: ['Test plan with V-Model mapping', 'Unit & integration tests (GoogleTest/Catch2)', 'System & acceptance testing', '70%+ line coverage target'],
    deliverable: 'Test plan, test code, results, coverage report, defect log',
    icon: '🧪',
  },
  {
    id: 6,
    name: 'Config. Management & QA',
    weeks: 'Week 6',
    marks: 6,
    gate: null,
    color: 'from-orange-500 to-orange-600',
    activities: ['Git branching strategy', 'CI pipeline (build + test on push)', 'Static analysis (cppcheck/clang-tidy)', 'Code metrics & self code review'],
    deliverable: 'Repo link, CI evidence, static analysis report, metrics',
    icon: '⚙️',
  },
  {
    id: 7,
    name: 'Deployment',
    weeks: 'Week 6',
    marks: 4,
    gate: null,
    color: 'from-rose-500 to-rose-600',
    activities: ['UML Deployment Diagram', 'Installation guide', 'User manuals for each role', 'Release notes (tag v1.0)'],
    deliverable: 'Deployment diagram, guides, manuals, release notes',
    icon: '🚀',
  },
  {
    id: 8,
    name: 'Maintenance',
    weeks: 'Week 7',
    marks: 6,
    gate: null,
    color: 'from-pink-500 to-pink-600',
    activities: ['CR1: Class-wide deadline extensions', 'Impact analysis', 'Implement as v1.1', 'Regression testing'],
    deliverable: 'CR analysis, v1.1 release, regression test results',
    icon: '🔧',
  },
];

export default function Timeline() {
  const [expandedPhase, setExpandedPhase] = useState<number | null>(null);

  return (
    <section id="timeline" className="py-16 lg:py-24">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h2 className="text-3xl sm:text-4xl font-bold text-white mb-3">Project Timeline</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            Waterfall model with V-Model verification. Each phase ends with a documented deliverable. 
            Phase gates require supervisor review before proceeding.
          </p>
        </div>

        <div className="relative">
          {/* Vertical line */}
          <div className="absolute left-4 md:left-1/2 top-0 bottom-0 w-0.5 bg-gradient-to-b from-indigo-500 via-purple-500 to-pink-500 transform md:-translate-x-1/2" />

          <div className="space-y-8">
            {phases.map((phase, index) => (
              <div
                key={phase.id}
                className={`relative flex items-start gap-4 md:gap-8 ${
                  index % 2 === 0 ? 'md:flex-row' : 'md:flex-row-reverse'
                }`}
              >
                {/* Timeline dot */}
                <div className="absolute left-4 md:left-1/2 w-4 h-4 rounded-full bg-gradient-to-r transform -translate-x-1/2 mt-6 z-10 ring-4 ring-slate-900"
                  style={{ backgroundImage: `linear-gradient(to right, var(--tw-gradient-stops))` }}
                >
                  <div className={`w-full h-full rounded-full bg-gradient-to-r ${phase.color}`} />
                </div>

                {/* Content card */}
                <div className={`ml-12 md:ml-0 md:w-[calc(50%-2rem)] ${index % 2 === 0 ? 'md:pr-8' : 'md:pl-8'}`}>
                  <button
                    onClick={() => setExpandedPhase(expandedPhase === phase.id ? null : phase.id)}
                    className="w-full text-left bg-white/5 backdrop-blur-sm rounded-xl border border-white/10 p-5 hover:bg-white/10 transition-all group"
                  >
                    <div className="flex items-center justify-between mb-2">
                      <div className="flex items-center gap-3">
                        <span className="text-2xl">{phase.icon}</span>
                        <div>
                          <h3 className="font-bold text-white group-hover:text-indigo-300 transition-colors">
                            Phase {phase.id}: {phase.name}
                          </h3>
                          <p className="text-xs text-slate-400">{phase.weeks}</p>
                        </div>
                      </div>
                      <div className="flex items-center gap-2">
                        {phase.gate && (
                          <span className="px-2 py-1 rounded-full bg-amber-500/20 text-amber-300 text-xs font-medium border border-amber-500/30">
                            {phase.gate}
                          </span>
                        )}
                        <span className="px-2 py-1 rounded-full bg-indigo-500/20 text-indigo-300 text-xs font-bold">
                          {phase.marks} marks
                        </span>
                      </div>
                    </div>

                    <div className={`overflow-hidden transition-all duration-300 ${expandedPhase === phase.id ? 'max-h-96 mt-4' : 'max-h-0'}`}>
                      <div className="space-y-3 pt-3 border-t border-white/10">
                        <div>
                          <h4 className="text-xs font-semibold text-slate-400 uppercase tracking-wider mb-2">Activities</h4>
                          <ul className="space-y-1.5">
                            {phase.activities.map((activity) => (
                              <li key={activity} className="text-sm text-slate-300 flex items-start gap-2">
                                <span className="text-indigo-400 mt-0.5">▸</span>
                                {activity}
                              </li>
                            ))}
                          </ul>
                        </div>
                        <div className="p-3 rounded-lg bg-indigo-500/10 border border-indigo-500/20">
                          <h4 className="text-xs font-semibold text-indigo-300 mb-1">📄 Deliverable</h4>
                          <p className="text-sm text-slate-300">{phase.deliverable}</p>
                        </div>
                      </div>
                    </div>

                    <div className="mt-2 text-xs text-slate-500 group-hover:text-slate-400 transition-colors">
                      {expandedPhase === phase.id ? 'Click to collapse' : 'Click to expand'}
                    </div>
                  </button>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
}
