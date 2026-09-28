import { useState } from 'react';

const functionalReqs = [
  { id: 'FR-01', desc: 'System shall authenticate users with username and password', priority: 'Must', actor: 'All' },
  { id: 'FR-02', desc: 'System shall enforce role-based access control (Admin, Lecturer, HoD, Student)', priority: 'Must', actor: 'All' },
  { id: 'FR-03', desc: 'Admin shall create, modify and delete user accounts', priority: 'Must', actor: 'Admin' },
  { id: 'FR-04', desc: 'Admin shall manage departments and courses', priority: 'Must', actor: 'Admin' },
  { id: 'FR-05', desc: 'Admin shall assign lecturers to courses and enrol students', priority: 'Must', actor: 'Admin' },
  { id: 'FR-06', desc: 'Lecturer shall create assignments with deadline and max mark', priority: 'Must', actor: 'Lecturer' },
  { id: 'FR-07', desc: 'Lecturer shall grant individual deadline extensions', priority: 'Must', actor: 'Lecturer' },
  { id: 'FR-08', desc: 'Lecturer shall mark submissions and write feedback', priority: 'Must', actor: 'Lecturer' },
  { id: 'FR-09', desc: 'HoD shall review and moderate marks for all courses in department', priority: 'Must', actor: 'HoD' },
  { id: 'FR-10', desc: 'HoD shall release approved marks to students', priority: 'Must', actor: 'HoD' },
  { id: 'FR-11', desc: 'Student shall view enrolled courses and assignments', priority: 'Must', actor: 'Student' },
  { id: 'FR-12', desc: 'Student shall submit assignment work', priority: 'Must', actor: 'Student' },
  { id: 'FR-13', desc: 'Student shall resubmit before deadline', priority: 'Must', actor: 'Student' },
  { id: 'FR-14', desc: 'Student shall view released marks and feedback', priority: 'Must', actor: 'Student' },
  { id: 'FR-15', desc: 'System shall apply late penalty (10%/day, max 3 days, then zero)', priority: 'Must', actor: 'System' },
  { id: 'FR-16', desc: 'System shall record all mark changes in audit log (who, when, old, new, reason)', priority: 'Must', actor: 'System' },
  { id: 'FR-17', desc: 'System shall track submission lifecycle (submitted → marked → moderated → released)', priority: 'Should', actor: 'System' },
  { id: 'FR-18', desc: 'System shall prevent students from viewing unmoderated marks', priority: 'Must', actor: 'Student' },
];

const nonFunctionalReqs = [
  { id: 'NFR-01', desc: 'System shall respond within 2 seconds for all operations with 800 students', priority: 'Must', category: 'Performance' },
  { id: 'NFR-02', desc: 'System shall work fully offline (no internet required)', priority: 'Must', category: 'Availability' },
  { id: 'NFR-03', desc: 'Passwords shall be stored as salted hashes (never plain text)', priority: 'Must', category: 'Security' },
  { id: 'NFR-04', desc: 'System shall use parameterised SQL queries to prevent injection', priority: 'Must', category: 'Security' },
  { id: 'NFR-05', desc: 'Interface shall be usable by non-IT-expert lecturers (max 3 clicks to common tasks)', priority: 'Should', category: 'Usability' },
  { id: 'NFR-06', desc: 'System shall maintain data integrity under concurrent access', priority: 'Must', category: 'Reliability' },
  { id: 'NFR-07', desc: 'Code shall follow documented coding standard and pass static analysis', priority: 'Should', category: 'Maintainability' },
  { id: 'NFR-08', desc: 'System shall support at least 800 concurrent student records', priority: 'Must', category: 'Scalability' },
];

const useCases = [
  { id: 'UC-01', name: 'Submit Assignment', actor: 'Student', type: 'Primary', complexity: 'Medium' },
  { id: 'UC-02', name: 'Mark Submission', actor: 'Lecturer', type: 'Primary', complexity: 'Medium' },
  { id: 'UC-03', name: 'Moderate & Release Marks', actor: 'HoD', type: 'Primary', complexity: 'High' },
  { id: 'UC-04', name: 'Manage Users', actor: 'Admin', type: 'Primary', complexity: 'Medium' },
  { id: 'UC-05', name: 'Create Assignment', actor: 'Lecturer', type: 'Primary', complexity: 'Medium' },
  { id: 'UC-06', name: 'Grant Extension', actor: 'Lecturer', type: 'Primary', complexity: 'Low' },
  { id: 'UC-07', name: 'Login', actor: 'All', type: 'Included', complexity: 'Low' },
  { id: 'UC-08', name: 'View Marks & Feedback', actor: 'Student', type: 'Primary', complexity: 'Low' },
  { id: 'UC-09', name: 'Apply Late Penalty', actor: 'System', type: 'Extend', complexity: 'Medium' },
  { id: 'UC-10', name: 'Resubmit Assignment', actor: 'Student', type: 'Extend', complexity: 'Low' },
  { id: 'UC-11', name: 'Audit Mark Changes', actor: 'System', type: 'Included', complexity: 'Low' },
  { id: 'UC-12', name: 'Manage Departments', actor: 'Admin', type: 'Primary', complexity: 'Low' },
];

const priorityColors: Record<string, string> = {
  Must: 'bg-red-500/20 text-red-300 border-red-500/30',
  Should: 'bg-amber-500/20 text-amber-300 border-amber-500/30',
  Could: 'bg-blue-500/20 text-blue-300 border-blue-500/30',
  "Won't": 'bg-slate-500/20 text-slate-300 border-slate-500/30',
};

const complexityColors: Record<string, string> = {
  Low: 'text-green-400',
  Medium: 'text-amber-400',
  High: 'text-red-400',
};

export default function Requirements() {
  const [activeTab, setActiveTab] = useState<'functional' | 'nonfunctional' | 'usecases'>('functional');

  return (
    <section id="requirements" className="py-16 lg:py-24 bg-white/[0.02]">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h2 className="text-3xl sm:text-4xl font-bold text-white mb-3">Requirements & Use Cases</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            SRS following ISO/IEC/IEEE 29148. MoSCoW prioritisation. Each requirement is uniquely identified and verifiable.
          </p>
        </div>

        {/* Tab switcher */}
        <div className="flex justify-center mb-8">
          <div className="bg-white/5 rounded-xl p-1 border border-white/10 inline-flex flex-wrap justify-center">
            <button
              onClick={() => setActiveTab('functional')}
              className={`px-4 py-2.5 rounded-lg text-sm font-medium transition-all ${
                activeTab === 'functional' ? 'bg-indigo-500 text-white' : 'text-slate-400 hover:text-white'
              }`}
            >
              ⚡ Functional ({functionalReqs.length})
            </button>
            <button
              onClick={() => setActiveTab('nonfunctional')}
              className={`px-4 py-2.5 rounded-lg text-sm font-medium transition-all ${
                activeTab === 'nonfunctional' ? 'bg-indigo-500 text-white' : 'text-slate-400 hover:text-white'
              }`}
            >
              🛡️ Non-Functional ({nonFunctionalReqs.length})
            </button>
            <button
              onClick={() => setActiveTab('usecases')}
              className={`px-4 py-2.5 rounded-lg text-sm font-medium transition-all ${
                activeTab === 'usecases' ? 'bg-indigo-500 text-white' : 'text-slate-400 hover:text-white'
              }`}
            >
              📋 Use Cases ({useCases.length})
            </button>
          </div>
        </div>

        {/* Content */}
        <div className="bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 overflow-hidden">
          {activeTab === 'functional' && (
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead>
                  <tr className="border-b border-white/10 bg-white/5">
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">ID</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Requirement</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Priority</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Actor</th>
                  </tr>
                </thead>
                <tbody>
                  {functionalReqs.map((req) => (
                    <tr key={req.id} className="border-b border-white/5 hover:bg-white/5 transition-colors">
                      <td className="px-4 py-3 font-mono text-indigo-300 text-xs">{req.id}</td>
                      <td className="px-4 py-3 text-slate-300">{req.desc}</td>
                      <td className="px-4 py-3">
                        <span className={`px-2 py-1 rounded-full text-xs border ${priorityColors[req.priority]}`}>
                          {req.priority}
                        </span>
                      </td>
                      <td className="px-4 py-3 text-slate-400">{req.actor}</td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}

          {activeTab === 'nonfunctional' && (
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead>
                  <tr className="border-b border-white/10 bg-white/5">
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">ID</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Requirement</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Priority</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Category</th>
                  </tr>
                </thead>
                <tbody>
                  {nonFunctionalReqs.map((req) => (
                    <tr key={req.id} className="border-b border-white/5 hover:bg-white/5 transition-colors">
                      <td className="px-4 py-3 font-mono text-indigo-300 text-xs">{req.id}</td>
                      <td className="px-4 py-3 text-slate-300">{req.desc}</td>
                      <td className="px-4 py-3">
                        <span className={`px-2 py-1 rounded-full text-xs border ${priorityColors[req.priority]}`}>
                          {req.priority}
                        </span>
                      </td>
                      <td className="px-4 py-3">
                        <span className="px-2 py-1 rounded-full text-xs bg-white/10 text-slate-300">
                          {req.category}
                        </span>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}

          {activeTab === 'usecases' && (
            <div className="overflow-x-auto">
              <table className="w-full text-sm">
                <thead>
                  <tr className="border-b border-white/10 bg-white/5">
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">ID</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Use Case</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Primary Actor</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Type</th>
                    <th className="text-left px-4 py-3 text-slate-400 font-medium">Complexity</th>
                  </tr>
                </thead>
                <tbody>
                  {useCases.map((uc) => (
                    <tr key={uc.id} className="border-b border-white/5 hover:bg-white/5 transition-colors">
                      <td className="px-4 py-3 font-mono text-indigo-300 text-xs">{uc.id}</td>
                      <td className="px-4 py-3 text-slate-300 font-medium">{uc.name}</td>
                      <td className="px-4 py-3 text-slate-400">{uc.actor}</td>
                      <td className="px-4 py-3">
                        <span className={`px-2 py-1 rounded-full text-xs ${
                          uc.type === 'Primary' ? 'bg-indigo-500/20 text-indigo-300' :
                          uc.type === 'Include' ? 'bg-green-500/20 text-green-300' :
                          'bg-amber-500/20 text-amber-300'
                        }`}>
                          {uc.type}
                        </span>
                      </td>
                      <td className="px-4 py-3">
                        <span className={`text-xs font-medium ${complexityColors[uc.complexity]}`}>
                          {uc.complexity}
                        </span>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>

        {/* Submission lifecycle */}
        <div className="mt-8 bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6">
          <h3 className="text-lg font-semibold text-white mb-4">🔄 Submission Life Cycle (State Machine)</h3>
          <div className="flex flex-wrap items-center justify-center gap-2">
            {[
              { state: 'Created', color: 'bg-slate-500' },
              { state: 'Submitted', color: 'bg-blue-500' },
              { state: 'Late', color: 'bg-amber-500' },
              { state: 'Marked', color: 'bg-purple-500' },
              { state: 'Moderated', color: 'bg-indigo-500' },
              { state: 'Released', color: 'bg-green-500' },
            ].map((s, i, arr) => (
              <div key={s.state} className="flex items-center gap-2">
                <div className={`${s.color} px-4 py-2 rounded-lg text-sm font-medium text-white`}>
                  {s.state}
                </div>
                {i < arr.length - 1 && (
                  <svg className="w-5 h-5 text-slate-500" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 5l7 7-7 7" />
                  </svg>
                )}
              </div>
            ))}
          </div>
          <p className="text-center text-xs text-slate-400 mt-4">
            Transitions: Submitted → Late (if past deadline) → Marked (by lecturer) → Moderated (by HoD) → Released (to student)
          </p>
        </div>
      </div>
    </section>
  );
}
