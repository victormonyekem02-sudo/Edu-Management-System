import { useState } from 'react';

const stakeholders = [
  {
    name: 'Principal',
    icon: '🏫',
    color: 'border-indigo-500/30 bg-indigo-500/5',
    interests: ['Simple interface for non-IT teachers', 'Students must not see marks before HoD moderation'],
    quote: '"Teachers are not IT experts, so the system must be simple. Students must not see any mark until it has been checked by the Head of Department."',
  },
  {
    name: 'Lecturer',
    icon: '👨‍🏫',
    color: 'border-purple-500/30 bg-purple-500/5',
    interests: ['Fast mark release to students', 'Ability to grant individual extensions for good reasons'],
    quote: '"I want students to get their marks as soon as I have marked the work. Sometimes a student has a good reason for needing more time."',
  },
  {
    name: 'Head of Department',
    icon: '🎓',
    color: 'border-amber-500/30 bg-amber-500/5',
    interests: ['Late penalty: -10%/day for 3 days, then zero', 'Must review all marks before release'],
    quote: '"Late work should lose 10% of the available mark for each day it is late, for up to three days. After that it should receive zero."',
  },
  {
    name: 'Student Representative',
    icon: '📚',
    color: 'border-blue-500/30 bg-blue-500/5',
    interests: ['Resubmission before deadline', 'Written feedback, not just numbers', 'Late work should not be simply rejected'],
    quote: '"Students should be able to fix a mistake and resubmit before the deadline, and we want written feedback, not just a number."',
  },
  {
    name: 'IT Officer',
    icon: '🖥️',
    color: 'border-green-500/30 bg-green-500/5',
    interests: ['Must work offline (old computers, unreliable internet)', 'Password protection', 'Fast response with 800 students'],
    quote: '"Our computers are old and the internet is unreliable, so the system must work offline. Passwords must be protected."',
  },
];

const conflicts = [
  {
    id: 1,
    title: 'Mark Release Timing',
    parties: 'Lecturer vs. Principal/HoD',
    lecturer: 'Wants immediate mark release after marking',
    other: 'Principal & HoD require moderation before release',
    resolution: 'Resolution: Marks go through a moderation workflow. Lecturer marks → HoD reviews/moderates → Marks released. This satisfies quality assurance while still being efficient.',
    color: 'border-red-500/20',
  },
  {
    id: 2,
    title: 'Late Submission Policy',
    parties: 'HoD vs. Student Rep',
    lecturer: 'HoD: Strict penalty (-10%/day, zero after 3 days)',
    other: 'Student: Late work should not be simply rejected',
    resolution: 'Resolution: Accept late submissions with automatic penalty (10%/day, max 3 days then zero). Extensions can be granted by lecturers for valid reasons. This balances fairness with academic rigour.',
    color: 'border-amber-500/20',
  },
  {
    id: 3,
    title: 'Resubmission Policy',
    parties: 'Student vs. System Constraints',
    lecturer: 'Students want to resubmit freely',
    other: 'Need to prevent abuse and maintain audit trail',
    resolution: 'Resolution: Allow resubmission before deadline only. Each resubmission is logged. After deadline, no further submissions unless an extension was granted.',
    color: 'border-blue-500/20',
  },
  {
    id: 4,
    title: 'System Complexity vs. Performance',
    parties: 'Principal vs. IT Officer',
    lecturer: 'Principal: System must be simple for non-IT teachers',
    other: 'IT Officer: Must work on old hardware with 800 students',
    resolution: 'Resolution: Console/simple GUI with efficient SQLite backend. Role-based menus keep interfaces simple. Local deployment avoids network dependency.',
    color: 'border-green-500/20',
  },
];

export default function Stakeholders() {
  const [activeTab, setActiveTab] = useState<'statements' | 'conflicts'>('statements');

  return (
    <section id="stakeholders" className="py-16 lg:py-24">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="text-center mb-12">
          <h2 className="text-3xl sm:text-4xl font-bold text-white mb-3">Stakeholder Analysis</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            Identifying stakeholders, their interests, and resolving conflicts in requirements.
          </p>
        </div>

        {/* Tab switcher */}
        <div className="flex justify-center mb-8">
          <div className="bg-white/5 rounded-xl p-1 border border-white/10 inline-flex">
            <button
              onClick={() => setActiveTab('statements')}
              className={`px-5 py-2.5 rounded-lg text-sm font-medium transition-all ${
                activeTab === 'statements' ? 'bg-indigo-500 text-white' : 'text-slate-400 hover:text-white'
              }`}
            >
              👥 Stakeholder Statements
            </button>
            <button
              onClick={() => setActiveTab('conflicts')}
              className={`px-5 py-2.5 rounded-lg text-sm font-medium transition-all ${
                activeTab === 'conflicts' ? 'bg-indigo-500 text-white' : 'text-slate-400 hover:text-white'
              }`}
            >
              ⚔️ Conflict Resolution
            </button>
          </div>
        </div>

        {activeTab === 'statements' ? (
          <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-4">
            {stakeholders.map((s) => (
              <div key={s.name} className={`rounded-xl border p-5 ${s.color} hover:scale-[1.02] transition-transform`}>
                <div className="flex items-center gap-3 mb-3">
                  <span className="text-3xl">{s.icon}</span>
                  <h3 className="font-bold text-white">{s.name}</h3>
                </div>
                <blockquote className="text-sm text-slate-300 italic mb-4 p-3 rounded-lg bg-black/20 border-l-2 border-indigo-500">
                  {s.quote}
                </blockquote>
                <div>
                  <h4 className="text-xs font-semibold text-slate-400 uppercase tracking-wider mb-2">Key Interests</h4>
                  <ul className="space-y-1.5">
                    {s.interests.map((interest) => (
                      <li key={interest} className="text-sm text-slate-300 flex items-start gap-2">
                        <span className="text-indigo-400">•</span>
                        {interest}
                      </li>
                    ))}
                  </ul>
                </div>
              </div>
            ))}
          </div>
        ) : (
          <div className="space-y-4">
            {conflicts.map((conflict) => (
              <div key={conflict.id} className={`rounded-xl border p-5 ${conflict.color} bg-white/5`}>
                <div className="flex items-center gap-3 mb-3">
                  <span className="w-8 h-8 rounded-full bg-indigo-500/20 flex items-center justify-center text-sm font-bold text-indigo-300">
                    {conflict.id}
                  </span>
                  <h3 className="font-bold text-white">{conflict.title}</h3>
                  <span className="ml-auto text-xs text-slate-400">{conflict.parties}</span>
                </div>
                <div className="grid md:grid-cols-2 gap-4 mb-4">
                  <div className="p-3 rounded-lg bg-red-500/5 border border-red-500/10">
                    <h4 className="text-xs font-semibold text-red-300 mb-1">Conflict</h4>
                    <p className="text-sm text-slate-300">{conflict.lecturer}</p>
                    <p className="text-sm text-slate-300 mt-1">{conflict.other}</p>
                  </div>
                  <div className="p-3 rounded-lg bg-green-500/5 border border-green-500/10">
                    <h4 className="text-xs font-semibold text-green-300 mb-1">✅ Resolution</h4>
                    <p className="text-sm text-slate-300">{conflict.resolution}</p>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </section>
  );
}
