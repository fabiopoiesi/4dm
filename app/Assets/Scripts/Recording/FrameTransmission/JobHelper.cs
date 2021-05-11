using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using Unity.Jobs;

namespace Reconstruction4D.Recording.FrameTransmission
{
    public static class JobHelper
    {
        public static void RunAsync(Action asyncMethod)
        {
            if (asyncMethod == null) throw new ArgumentNullException("asyncMethod");

            var prevCtx = SynchronizationContext.Current;
            try
            {
                var syncCtx = new UnitySynchronizationContext(true);
                SynchronizationContext.SetSynchronizationContext(syncCtx);

                syncCtx.OperationStarted();
                asyncMethod();
                syncCtx.OperationCompleted();

                syncCtx.RunOnCurrentThread();
            }
            finally { SynchronizationContext.SetSynchronizationContext(prevCtx); }
        }

        public static void RunAsync(Func<Task> asyncMethod)
        {
            if (asyncMethod == null) throw new ArgumentNullException("asyncMethod");

            var prevCtx = SynchronizationContext.Current;
            try
            {
                var syncCtx = new UnitySynchronizationContext(false);
                SynchronizationContext.SetSynchronizationContext(syncCtx);

                var t = asyncMethod();
                if (t == null) throw new InvalidOperationException("No task provided.");
                t.ContinueWith(delegate { syncCtx.Complete(); }, TaskScheduler.Default);

                syncCtx.RunOnCurrentThread();
                t.GetAwaiter().GetResult();
            }
            finally { SynchronizationContext.SetSynchronizationContext(prevCtx); }
        }

        public static T RunAsync<T>(Func<Task<T>> asyncMethod)
        {
            if (asyncMethod == null) throw new ArgumentNullException("asyncMethod");

            var prevCtx = SynchronizationContext.Current;
            try
            {
                var syncCtx = new UnitySynchronizationContext(false);
                SynchronizationContext.SetSynchronizationContext(syncCtx);

                var t = asyncMethod();
                if (t == null) throw new InvalidOperationException("No task provided.");
                t.ContinueWith(delegate { syncCtx.Complete(); }, TaskScheduler.Default);

                syncCtx.RunOnCurrentThread();
                return t.GetAwaiter().GetResult();
            }
            finally { SynchronizationContext.SetSynchronizationContext(prevCtx); }
        }

        public static JobHandleAwaiter GetAwaiter(this JobHandle jobHandle) => new JobHandleAwaiter(jobHandle);

        public static JobHandleAwaiter ScheduleAsync<T>(this T job) where T : struct, IJob
        {
            var unitySC = SynchronizationContext.Current as UnitySynchronizationContext;

            if (unitySC == null)
                throw new InvalidOperationException("Awaiting jobs must be done in the UnitySynchronizationContext");

            var previousHandle = unitySC.CurrentHandle;

            var handle = job.Schedule(previousHandle);

            unitySC.CurrentHandle = handle;

            return new JobHandleAwaiter(handle);
        }

        public static JobHandleAwaiter ScheduleAsync<T>(this T job, int arrayLength, int innerloopBatchCount) where T : struct, IJobParallelFor
        {
            var unitySC = SynchronizationContext.Current as UnitySynchronizationContext;

            if (unitySC == null)
                throw new InvalidOperationException("Awaiting jobs must be done in the UnitySynchronizationContext");

            var previousHandle = unitySC.CurrentHandle;

            var handle = job.Schedule(arrayLength, innerloopBatchCount, previousHandle);

            unitySC.CurrentHandle = handle;

            return new JobHandleAwaiter(handle);
        }

        private sealed class UnitySynchronizationContext : SynchronizationContext
        {
            private readonly BlockingCollection<KeyValuePair<SendOrPostCallback, object>> m_queue =
                new BlockingCollection<KeyValuePair<SendOrPostCallback, object>>();
            private int m_operationCount = 0;
            private readonly bool m_trackOperations;

            public UnitySynchronizationContext(bool trackOperations)
            {
                m_trackOperations = trackOperations;
            }

            public JobHandle CurrentHandle { get; set; } = default(JobHandle);

            public override void Post(SendOrPostCallback d, object state)
            {
                m_queue.Add(new KeyValuePair<SendOrPostCallback, object>(d, state));
            }

            public override void Send(SendOrPostCallback d, object state)
            {
                throw new NotSupportedException("Synchronously sending is not supported.");
            }

            public void RunOnCurrentThread()
            {
                foreach (var workItem in m_queue.GetConsumingEnumerable())
                    workItem.Key(workItem.Value);
            }

            public void Complete() { m_queue.CompleteAdding(); }

            public override void OperationStarted()
            {
                if (m_trackOperations)
                    Interlocked.Increment(ref m_operationCount);
            }

            public override void OperationCompleted()
            {
                if (m_trackOperations &&
                    Interlocked.Decrement(ref m_operationCount) == 0)
                    Complete();
            }
        }

        public struct JobHandleAwaiter : INotifyCompletion
        {
            readonly JobHandle jobHandle;
            public JobHandleAwaiter(JobHandle jobHandle)
            {
                this.jobHandle = jobHandle;
            }

            public JobHandleAwaiter GetAwaiter() => this;

            public bool IsCompleted => jobHandle.IsCompleted;

            public void OnCompleted(Action continuation)
            {
                jobHandle.Complete();
                continuation();
            }

            public void GetResult() { }
        }
    }
}
